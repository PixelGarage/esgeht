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

define('POST_WEBFORM_ID', 18);

/**
 * Defines the available multi-step webform process(es).
 *
 * The process contains a postcard data collection step and a postcard delivery step.
 */
function pixelgarage_postcard_multi_step_webform_info_alter(&$info) {
  //
  // create the steps with its specific parameters
  $info['post data'] = array(
    // webform node id
    'nid' =>  POST_WEBFORM_ID,
    // previous step
    'prev step' => null,
    // next step id
    'next step' => null,
    // submission related node type
    'related node type' => 'post'
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
    case 'post data':
      _post_data_step_presave($step_options, $node, $submission);
      break;
  }

}

function _post_data_step_presave(&$step_options, $node, &$submission) {
  //
  // POST DATA SUBMISSION PRESAVE:
  // create or update submission related node with submitted values
  $master = postcard_webform_master_form($node);
  $status = (!empty($master->field_publish_post_immediately) && $master->field_publish_post_immediately[LANGUAGE_NONE][0]['value'] == 1) ? 1 : 0;

  //
  // check if a post exists already, and update or create the post accordingly
  $post_nid = _get_submission_value($master, $submission, 'post_nid');

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
  $post->title = t('It works for @you', array('@you' => $submission->data[17][0])); // create node title
  $post->field_e_mail[$post->language][0]['email'] = $submission->data[19][0]; // email
  $post->field_image[$post->language][0]['fid'] = $submission->data[1][0]; // fid image
  $post->field_category[$post->language][0]['tid'] = $submission->data[20][0]; // Category
  $post->field_your_name[$post->language][0]['value'] = $submission->data[17][0]; // your name
  $post->field_quote[$post->language][0]['value'] = substr($submission->data[5][0], 0, 100); // Quote
  $post = node_submit($post);
  node_save($post);

  //
  // add related post id to submission
  _set_submission_value($master, $submission, 'post_nid', $post->nid);
}

/**
 * Helper function to get submission value for given form key.
 */
function _get_submission_value($master, $submission, $form_key) {
  foreach ($master->webform['components'] as $key => $data) {
    if ($data['form_key'] == $form_key) {
      return isset($submission->data[$key]) ? $submission->data[$key][0] : false;
    }
  }
}

/**
 * Helper function to set submission value for given form key.
 */
function _set_submission_value($master, $submission, $form_key, $value) {
  foreach ($master->webform['components'] as $key => $data) {
    if ($data['form_key'] == $form_key) {
      $submission->data[$key][0] = $value;
      break;
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
    case 'post data':
      // unpublish post (no deletion to keep postcard information for delivery step)
      $post_nid = _get_submission_value($master, $submission, 'post_nid');
      if ($post_nid) {
        $post = node_load($post_nid);
        $post->status = 0;
        node_save($post);
      }
      break;
  }

}


