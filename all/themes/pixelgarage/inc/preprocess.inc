<?php
/**
 * Created by PhpStorm.
 * User: ralph
 * Date: 06.02.15
 * Time: 17:23
 */

/**
 * Preprocess variables for the html template.
 */
function pixelgarage_preprocess_html(&$vars) {
  // make sure jQuery UI and effects is loaded for anonymous users
  drupal_add_library('system', 'ui');
  drupal_add_library('system', 'effects');

}

/**
 * Override or insert variables for the page templates.
 */
function pixelgarage_preprocess_page(&$vars) {
  // hide titles on login forms
  pg_login_preprocess_page($vars);

  // replace logo with .svg
  //$vars['logo'] = str_replace('.jpg', '.svg', $vars['logo']);
}

/**
 * Add specific category classes to all category fields.
 */
function pixelgarage_preprocess_field(&$vars) {
  //
  // add icons to category fields
  $element = $vars['element'];

  if ($element['#field_name'] === 'field_category' || $element['#field_name'] === 'field_category_multiple') {
    foreach($element['#items'] as $index => $item) {
      $vars['items_classes'][$index] = 'category-' . $element['#items'][$index]['tid'];
    }
  }
}


/* =============================================================================
 *      Exposed form
 * ========================================================================== */
/**
 * Implements template_preprocess_views_exposed_form().
 *
 * Adds total result count to exposed form and the placeholder for the search field.
 */
function pixelgarage_preprocess_views_exposed_form(&$vars) {
  if ($filter = &$vars['widgets']['filter-combine']) {
    $placeholder = 'placeholder="' . $filter->label . '"/>';
    $filter->widget = str_replace('/>', $placeholder, $filter->widget);
  }
}


/* =============================================================================
 *      Rating widget
 * ========================================================================== */

function pixelgarage_preprocess_rate_template_thumbs_up(&$vars) {
  $results = $vars['results'];
  $vars['score'] = $results['count'];
  $vars['info'] = null;
}



/* =============================================================================
 *      Front Masonry
 * ========================================================================== */
/**
 * Adapts the proximity view, so that always two randomly chosen testimonials are visible
 * in the masonry.
 */
function pixelgarage_preprocess_views_view_proximity (&$vars) {
  //
  // do not inject SOME items when filtering
  if (!empty($vars['view']->exposed_data['combine'])) return;

  _random_testimonials($vars);
  _social_items($vars);
}

/**
 * Creates an associative array with randomly chosen testimonials to be added at random places in the view.
 */
function _random_testimonials(&$vars) {
  static $testimonials = array();
  if (empty($testimonials)) {
    global $language;
    // get the published testimonials
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', 'testimonial')
      ->propertyCondition('status', NODE_PUBLISHED)
      ->propertyCondition('language', $language->language);
    $result = $query->execute();
    if ($result && !empty($result['node'])) {
      $testimonials = node_load_multiple(array_keys($result['node']));
    }
    else return;
  }

  // generic testimonial link
  global $language;
  switch ($language->language) {
    case 'de':
      $default_link = 'http://www.gruenewirtschaft.ch';
      break;
    case 'fr':
      $default_link = 'http://www.economieverte.ch';
      break;
  }

  // chose random testimonial from list
  $view = $vars['view'];
  $row_count = count($view->result) - 1;
  $keys = array_keys($testimonials);
  foreach ($keys as $index => $key) {
    $testimonial_link = !empty($testimonials[$key]->field_link) ? $testimonials[$key]->field_link[LANGUAGE_NONE][0]['url'] : $default_link;
    $i = mt_rand(0, $row_count);
    $testimonial = node_view($testimonials[$key], 'teaser');
    $vars['testimonials'][$i] = array (
      'rendered_testimonial' => render($testimonial),
      'testimonial_link' => $testimonial_link,
    );
  }
}

function _social_items(&$vars) {
  //
  // get the SOME items of last cron run
  $session_data = &pixelgarage_session_data();
  $social_items = array();
  $timestamp = time();
  if (isset($session_data['$social_items']) && isset($session_data['social_timestamp']) &&
    ($timestamp - $session_data['social_timestamp']) < 300) {
    $social_items = $session_data['$social_items'];
  }
  else {
    $view_object = views_get_view("some_posts");
    $view_object->set_display("block");
    $view_object->pre_execute();
    $view_object->execute();
    $view_object->render();

    $rendered_fields = $view_object->style_plugin->rendered_fields;
    foreach ($rendered_fields as $index => $rendered_field) {
      $rendered_field['nid'] = $view_object->result[$index]->nid;
      $social_items[$index] = $rendered_field;
    }
    $session_data['$social_items'] = $social_items;
    $session_data['social_timestamp'] = $timestamp;
  }

  //
  // create random SOME array
  $view = $vars['view'];
  $total_posts = $view->query->pager->total_items;
  $current_page = $view->query->pager->current_page;
  $posts_per_page = (int)$view->query->pager->options['items_per_page'];

  $total_items = count($social_items);
  $social_items_per_page = 5;
  $start_index = $current_page * $social_items_per_page;
  $end_index =  ($current_page + 1) * $social_items_per_page - 1;

  // get items for current page
  $items = array();
  $keys = _distinct_random_numbers(0, $posts_per_page-1, $social_items_per_page);
  foreach ($keys as $index => $key) {
    if ($total_items > ($start_index + $index)) {
      $items[$key] = $social_items[$start_index + $index];
    }
  }
  $vars['social_items'] = $items;
  $vars['social_items_rest'] = array();

  // last page, add rest of items
  if ($total_posts < (($current_page + 1) * $posts_per_page)) {
    $vars['social_items_rest'] = array_slice($social_items, $end_index+1, null, true);
  }

}


function _distinct_random_numbers($min, $max, $count) {
  $result = array();
  $range = $max - $min;
  if ($range < $count) {
    for ($i = 0; $i < $range; $i++) {
      $result[$i] = '';
    }
  }
  else {
    while (count($result) < $count) {
      $i = mt_rand($min, $max);
      $result[$i] = '';
    }
  }
  return array_keys($result);
}
