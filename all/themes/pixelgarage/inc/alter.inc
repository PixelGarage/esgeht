<?php
/**
 * Created by PhpStorm.
 * User: ralph
 * Date: 06.02.15
 * Time: 17:22
 */

/**
 * Alters the menu entries.
 * @param $items
 */
function pixelgarage_menu_alter(&$items) {
  // hides local task tabs
  pg_login_menu_alter($items);
}


/* =============================================================================
 *    Proximity alter hooks implementation
 * ========================================================================== */

/**
 * Alters the proximity item load parameter array.
 *
 * The load parameter array defines a specific url parameter for each proximity item.
 * This parameter is added at the end of the request url and must be unique and URL conform.
 * The unique parameter defines, which item content has to be loaded from
 * the server (see next api function).
 *
 * @param $container_index      int     Index of proximity container (if more than one container exists in one page).
 * @param $view                 array   The view.
 * @param $ajax_load_params     array   Array of ajax load parameters to be altered, one for each proximity item (row)
 *                                      retrieved by the view. Default is the views row index.
 */
function pixelgarage_proximity_load_params_alter($container_index, $view, &$ajax_load_params) {
  // Return the node id as ajax parameter for each item.
  foreach ($view->result as $id => $item) {
    $ajax_load_params[$id] = $item->nid;
  }
}

/**
 * Implements the item specific content as render array or html string.
 * The input parameter $param contains the unique load parameter of the requested item.
 *
 * @param $container_index      int     Index of proximity container (if more than one container exists in one page).
 * @param $param                string  The item specific load parameter (see also hook_proximity_ajax_load_params_alter).
 * @param $render_item          mixed   The rendered content to be returned to the client. The $render_item should be
 *                                      replaced either by a string (rendered html content), a render array or an integer (error code).
 */
function pixelgarage_proximity_render_item_alter($container_index, $param, &$render_item) {
  // return the render array for the specific node, if available
  global $language;
  if ($node= node_load($param)) {
    $view_mode = 'full';
    if (property_exists($node, 'ds_switch') && !empty($node->ds_switch)) {
      // take an alternate view mode set by the ds switch
      $view_mode = $node->ds_switch;
    }
    $render_item = node_view($node, $view_mode, $language->language);
  }
}


/* =============================================================================
 *     Multi-step webform hooks
 * ========================================================================== */

define('POSTCARD_DATA_WEBFORM_ID', 18);

define('POSTCARD_DELIVERY_WEBFORM_ID', 789);

/**
 * Defines the available multi-step webform process(es).
 *
 * The process contains a postcard data collection step and a postcard delivery step.
 */
function pixelgarage_postcard_multi_step_webform_info_alter(&$info) {
  //
  // create the steps with its specific parameters
  $info['postcard data'] = array(
    // webform node id
    'nid' =>  POSTCARD_DATA_WEBFORM_ID,
    // previous step
    'prev step' => null,
    // next step id
    'next step' => 'postcard delivery',
    // submission related node type
    'related node type' => 'post'
  );
  $info['postcard delivery'] = array(
    // delivery webform node id
    'nid' =>  POSTCARD_DELIVERY_WEBFORM_ID,
    // previous step
    'prev step' => 'postcard data',
    // next step id
    'next step' => null,
  );

  return $info;
}

/**
 * Alter a specific step submission of a multi-step webform,  prior to saving it in the database.
 *
 * @param $step_options array
 *    An array holding all step options to be transferred between the steps. This array can be altered between steps.
 *    The initial array holds two parameters:
 *      step_key: The key of the step.
 *      step_info: The step info array.
 * @param $node     object
 *    The webform node of the particular step
 * @param $submission   object
 *    The submission of the particular step
 */
function pixelgarage_postcard_multi_step_submission_presave_alter(&$step_options, $node, &$submission) {
  //
  // process the multi-step webform submissions according to the step
  $step = $step_options['step_key'];
  switch ($step) {
    case 'postcard data':
      _post_data_step_presave($step_options, $node, $submission);
      break;
    case 'postcard delivery':
      _post_delivery_step_presave($step_options, $node, $submission);
      break;
  }

}

function _post_data_step_presave(&$step_options, $node, &$submission) {
  //
  // POSTCARD DATA SUBMISSION PRESAVE:
  // create or update submission related node with submitted values
  $master = postcard_webform_master_form($node);
  $status = (!empty($master->field_publish_post_immediately) && $master->field_publish_post_immediately[LANGUAGE_NONE][0]['value'] == 1) ? 1 : 0;

  //
  // check if a post exists already, and update or create the post accordingly
  $post_nid = false;
  foreach ($master->webform['components'] as $key => $data) {
    if ($data['form_key'] == 'post_nid') {
      $post_nid = !empty($submission->data[$key]) ? $submission->data[$key][0] : false;
    }
  }

  if ($post_nid) {
    // update existing post
    $post = node_load($post_nid);
    $post->status = $status;

  } else {
    // create post node and save it in the submission
    $post = new stdClass();
    $post->type = $step_options['step_info']['related node type'];
    $post->language = LANGUAGE_NONE;
    $post->uid = 1;
    $post->status = $status;
    $post->comment = 0;
    $post->promote = 0;
    node_object_prepare($post);

  }
  //
  // update submission data on the post and save it
  $post->title = t('@you for @name', array('@you' => $submission->data[17][0], '@name' => $submission->data[4][0])); // create node title
  $post->field_e_mail[$post->language][0]['email'] = $submission->data[19][0]; // email
  $post->field_name[$post->language][0]['value'] = $submission->data[4][0]; // child name
  $post->field_your_name[$post->language][0]['value'] = $submission->data[17][0]; // your name
  $post->field_quote[$post->language][0]['value'] = substr($submission->data[5][0], 0, 100); // Quote
  $post->field_image[$post->language][0]['fid'] = $submission->data[1][0]; // fid image
  $post = node_submit($post);
  node_save($post);

  //
  // add step options about created post
  $step_options['post_nid'] = $post->nid;
  $step_options['post_status'] = $status;

  //
  // create postcard and high resolution image and add it to post node, if available
  $pdf_name = $post->title . '_' . time() . '.pdf';
  $options = array(
    'post_node' => $post,
  );
  $pdf_uri = postcard_create_pdf($pdf_name, $options);

  if ($pdf_uri) {
    //
    // delete old pdf, if any
    $old_pdf_fid = !empty($post->field_postcard) ? $post->field_postcard[$post->language][0]['fid'] : null;
    if (isset($old_pdf_fid)) {
      $old_pdf_file = file_load($old_pdf_fid);
      if ($old_pdf_file) {
        file_delete($old_pdf_file);
      }
    }

    //
    // manage created pdf file in post
    $pdf_file = new stdClass();
    $pdf_file->fid = NULL;
    $pdf_file->uri = $pdf_uri;
    $pdf_file->filename = drupal_basename($pdf_uri);
    $pdf_file->filemime = file_get_mimetype($pdf_uri);
    $pdf_file->status   = FILE_STATUS_PERMANENT;
    $pdf_file->uid      = 1;
    $pdf_file->display  = 1;
    $pdf_file = file_save($pdf_file);

    $post->field_postcard[$post->language][0]['fid'] = $pdf_file->fid;
    $post->field_postcard[$post->language][0]['display'] = $pdf_file->display;
    $step_options['postcard_fid'] = $pdf_file->fid;

    // manage created hires image in node, if any
    if (isset($options['postcard_image_uri'])) {
      //
      // delete old image, if any
      $old_img_fid = !empty($post->field_hires_image) ? $post->field_hires_image[$post->language][0]['fid'] : null;
      if (isset($old_img_fid)) {
        $old_img_file = file_load($old_img_fid);
        if ($old_img_file) {
          file_delete($old_img_file);
        }
      }

      //
      // manage created image file in post
      $pdf_img_uri = $options['postcard_image_uri'];
      $pdf_img = new stdClass();
      $pdf_img->fid = NULL;
      $pdf_img->uri = $pdf_img_uri;
      $pdf_img->filename = trim(drupal_basename($pdf_img_uri));
      $pdf_img->filemime = file_get_mimetype($pdf_img_uri);
      $pdf_img->status   = FILE_STATUS_PERMANENT;
      $pdf_img = file_save($pdf_img);

      $post->field_hires_image[$post->language][0]['fid'] = $pdf_img->fid;
      $step_options['postcard_img_fid'] = $pdf_img->fid;
    }
    node_save($post);

  }
  
  //
  // add related post id to submission
  foreach ($master->webform['components'] as $key => $data) {
    if ($data['form_key'] == 'post_nid') {
      $submission->data[$key][0] = $post->nid;
      break;
    }
  }

  //
  // add postcard to next webform node
  $transl_nodes = translation_node_get_translations(POSTCARD_DELIVERY_WEBFORM_ID);
  $postcard_img_fid = isset($step_options['postcard_img_fid']) ? $step_options['postcard_img_fid'] : null;
  foreach ($transl_nodes as $transl_node) {
    $next_node = node_load($transl_node->nid);
    $next_node->field_image_postcard[LANGUAGE_NONE][0]['fid'] = $postcard_img_fid;
    node_save($next_node);
  }

}

function _post_delivery_step_presave(&$step_options, $node, &$submission) {
  //
  // POSTCARD DELIVERY SUBMISSION PRESAVE:
  // multi-step form is finished, publish post
  if (isset($step_options['post_nid'])) {
    //
    // add post id and name to submission
    $post = node_load($step_options['post_nid']);
    $master = postcard_webform_master_form($node);
    foreach ($master->webform['components'] as $key => $data) {
      if ($data['form_key'] == 'post_nid') {
        $submission->data[$key][0] = $post->nid;
        continue;
      }
      if ($data['form_key'] == 'full_name') {
        if (empty($submission->data[$key][0])) {
          $submission->data[$key][0] = $post->field_your_name[$post->language][0]['value'];
          continue;
        }
      }
      if ($data['form_key'] == 'postcard_pdf') {
        if (empty($submission->data[$key][0])) {
          $submission->data[$key][0] = $post->field_postcard[$post->language][0]['fid'];
          continue;
        }
      }
    }
  }
}

/**
 * Alter a specific step removal of a multi-step webform, prior of the submission deletion, e.g. delete objects
 * you have created in the submission presave step.
 *
 * @param $step_key   string
 *    The step key indicating which submission of the multi-step webform is going to be deleted
 * @param $node   object
 *    The webform node (tnid)
 * @param $submission  object
 *    The submission object.
 */
function pixelgarage_postcard_multi_step_submission_remove_alter($step_key, $node, $submission) {
  //
  // remove step specific submitted data first
  $master = postcard_webform_master_form($node);
  switch ($step_key) {
    case 'postcard data':
      // unpublish post (no deletion to keep postcard information for delivery step)
      $post_nid = false;
      foreach ($master->webform['components'] as $key => $data) {
        if ($data['form_key'] == 'post_nid') {
          $post_nid = $submission->data[$key][0];
          break;
        }
      }
      if ($post_nid) {
        $post = node_load($post_nid);
        $post->status = 0;
        node_save($post);
      }
      break;
    case 'postcard delivery':
      // nothing to do
      break;
  }

}



/* =============================================================================
 *     Postcard hooks
 * ========================================================================== */

/**
 * Allows to draw to a fully initialised pdf. Only the postcard itself has to be drawn here.
 * The initialisation and storage of the TCPDF object is done in the postcard_create_pdf() function.
 * (See https://tcpdf.org for more details about PDF creation.)
 *
 * Additionally the PDF postcard can be converted (ImageMagick need to be installed) to a high resolution image,
 * if an image file path with an image type extension is returned.
 *
 * @param $tcpdf_obj  object
 *    A fully initialized TCPDF object. Use this object to draw your pdf.
 * @param $pdf_uri string
 *    Alter the predefined pdf uri (schema://target.pdf), if needed.
 * @param $options array
 *    The options array passed to the postcard_create_pdf() function.
 *
 *    The options array has an additional parameter added.
 *    options['postcard_image_type']:
 *      Indicates the file type of the high resolution postcard image to be created from the pdf (default .png).
 *      You can alter the predefined image type, e.g. '.jpg', '.png' etc. or set it to FALSE to prevent image creation.
 *
 *    Image creation prerequisites:
 *      ImageMagick has to be installed (convert binary)
 */
function pixelgarage_postcard_draw_pdf_alter(&$tcpdf_obj, &$pdf_uri, &$options) {
  //
  // get post node from options
  $post = $options['post_node'];

  //
  // create hires image from node image (pdf resolution), if possible
  $image_fid = $post->field_image[$post->language][0]['fid'];
  $file_img = file_load($image_fid);

  $style = image_style_load('post_hires_image');
  $img_style_uri = image_style_path($style['name'], $file_img->uri);
  if (!image_style_create_derivative($style, $file_img->uri, $img_style_uri)) {
    // hires image could not be created, take lowres image
    $img_style_uri = image_style_path('post_image', $file_img->uri);
  }
  $img_style_sw = file_stream_wrapper_get_instance_by_uri($img_style_uri);
  $img_style_path = $img_style_sw->realpath();

  //
  // get banner image path from theme
  global $language, $base_url;
  switch($language->language) {
    case 'de':
    default:
      $ext = '_d.png';
      break;
    case 'fr':
      $ext = '_f.png';
      break;
    case 'en':
      $ext = '_e.png';
      break;
  }
  $banner_img_path = drupal_get_path('theme', 'pixelgarage') . '/images/logo_banner' . $ext;

  //
  // get gradient image
  $gradient_img_path = drupal_get_path('theme', 'pixelgarage') . '/images/gradient_bg.png';

  //
  // get strings from post
  $slogan = t('Nachhaltig bis 2050');
  $name = $post->field_name[$post->language][0]['value'];
  $quote = $post->field_quote[$post->language][0]['value'];
  $your_name = '- ' . $post->field_your_name[$post->language][0]['value'] . ' -';


  //
  // Draw the postcard
  //
  // set pdf specifc settings
  $tcpdf_obj->SetCreator(PDF_CREATOR);
  $tcpdf_obj->SetAuthor('FürDich.org');
  $tcpdf_obj->SetTitle('Postcard created by FürDich.org');
  $tcpdf_obj->SetSubject('Ja zur Grünen Wirtschaft');
  $tcpdf_obj->SetKeywords('Postcard, Grüne Wirtschaft, tcpdf, PDF, Drupal, Pixelgarage');
  $tcpdf_obj->SetMargins(0, 0, 0);

  // disable auto-page-break status
  $bMargin = $tcpdf_obj->getBreakMargin();
  $tcpdf_obj->SetAutoPageBreak(false, 0);

  // line and fill color
  //$tcpdf_obj->SetFillColor(128, 128, 128);
  //$tcpdf_obj->SetDrawColor(128);
  $tcpdf_obj->SetLineWidth(0.8);
  $line_style = array('width' => 0.6, 'cap' => 'round', 'join' => 'miter', 'dash' => 0, 'color' => array(255, 255, 255));

  // fill background with green
  $bg_color = array(155, 195, 27); // green (#9bc31b)
  $tcpdf_obj->Rect(0, 0, 105, 148, 'F', array(), $bg_color);

  // draw post image (high-resolution)
  $tcpdf_obj->Image($img_style_path, 0, 0, 105, 105, '', '', '', false, 300, '', false, false, 0, 'CT');

  // draw banner image (418x161px)
  $tcpdf_obj->Image($banner_img_path, 5.25, 82.88, 94.5, 0, '', '', '', false, 300, '', false, false, 0);

  // draw linear gradient background
  $tcpdf_obj->Image($gradient_img_path,0, 0, 105, 0, '', '', '', false, 300, '', false, false, 0);


  // draw slogan with dashes
  $tcpdf_obj->Line(5, 7, 10, 7, $line_style);
  $tcpdf_obj->Line(95, 7, 100, 7, $line_style);
  $tcpdf_obj->addFont('museosans_900', '', 'museosans_900');
  $tcpdf_obj->setFont('museosans_900');//, null, 'museosans_900', false);
  $tcpdf_obj->setFontSize(22);
  $tcpdf_obj->SetTextColor(255, 255, 255);
  $tcpdf_obj->setXY(0,2);
  // Format: Cell($w, $h=0, $txt='', $border=0, $ln=0, $align='', $fill=false, $link='', $stretch=0, $ignore_min_height=false, $calign='T', $valign='M')
  $tcpdf_obj->Cell(0, 0, $slogan, 0, 0, 'C', false);

  // draw quote
  // Format: writeHTML($html, $ln=true, $fill=false, $reseth=false, $cell=false, $align='')
  // Format: writeHTMLCell($w, $h, $x, $y, $html='', $border=0, $ln=0, $fill=false, $reseth=true, $align='', $autopadding=true)
  $tcpdf_obj->addFont('museosans_900', '', 'museosans_900');
  $tcpdf_obj->setFont('museosans_900', 'B');//, null, 'museosans_900', false);
  $tcpdf_obj->setFontSize(17);
  $tcpdf_obj->SetTextColor(255, 255, 255);
  $tcpdf_obj->setCellHeightRatio(1.1);
  $tcpdf_obj->setXY(5.25,118.4);
  // Format: MultiCell($w, $h, $txt, $border=0, $align='J', $fill=false, $ln=1, $x='', $y='', $reseth=true, $stretch=0, $ishtml=false, $autopadding=true, $maxh=0, $valign='T', $fitcell=false)
  $tcpdf_obj->MultiCell(94.5, 17, $quote, 0, 'C', false, 1);

  // draw your name
  $tcpdf_obj->addFont('MuseoSans_300', '', 'museosans_300');
  $tcpdf_obj->setFont('MuseoSans_300', '', null, 'museosans_300', false);
  $tcpdf_obj->setFontSize(10);
  $tcpdf_obj->SetTextColor(255, 255, 255);
  $tcpdf_obj->setXY(0,140);
  // Format: Cell($w, $h=0, $txt='', $border=0, $ln=0, $align='', $fill=false, $link='', $stretch=0, $ignore_min_height=false, $calign='T', $valign='M')
  $tcpdf_obj->Cell(0, 0, $your_name, 0, 0, 'C', false);

  // draw child name
  $tcpdf_obj->addFont('Pacifico', '', 'pacifico');
  $tcpdf_obj->setFont('Pacifico');
  $tcpdf_obj->SetTextColor(155, 195, 27); // green (#9bc31b)
  $tcpdf_obj->setFontSize(24);
  $tcpdf_obj->StartTransform();
  $tcpdf_obj->setXY(5.25,103);
  $tcpdf_obj->Rotate(2);
  // Format: Cell($w, $h=0, $txt='', $border=0, $ln=0, $align='', $fill=false, $link='', $stretch=0, $ignore_min_height=false, $calign='T', $valign='M')
  $tcpdf_obj->Cell(94.5, 0, $name, 0, 0, 'C', false);
  // rotate the text
  $tcpdf_obj->StopTransform();


  // restore auto-page-break status
  $tcpdf_obj->SetAutoPageBreak(true, $bMargin);
  $tcpdf_obj->setPageMark();

}
