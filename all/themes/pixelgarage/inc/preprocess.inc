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
function pixelgarage_preprocess_views_view(&$vars) {
}


/**
 * Adapts the proximity view, so that always two randomly chosen testimonials are visible
 * in the masonry.
 */
function pixelgarage_preprocess_views_view_proximity (&$vars) {
  //
  // check all filters
  $exposed_data = $vars['view']->exposed_data;
  $search_filter = !empty($exposed_data['combine']);
  $category_filter = $exposed_data['field_category_tid'] != 'All';
  $sort_by_rating = $exposed_data['sort_by'] != 'created';

  //
  // show only tech posts when techFilter is enabled
  if ($exposed_data['field_category_tid'] == 56) {
    $special_testimonials = _get_linked_testimonials(true);
    $testimonials = _get_linked_testimonials();
    $vars['testimonials'] = array_merge($special_testimonials, $testimonials);
    $count = count($vars['testimonials']);

    // create dummy rows and call proximity view
    $vars['rows'] = array_fill(0, $count, null);
  }

  //
  // do not inject tech posts or SOME items, when filter is active
  if ($search_filter || $category_filter || $sort_by_rating) return;

  //
  // define numbers of injected items
  $number_of_testimonials_per_page = 3;
  $number_of_specials_per_page = 3;
  $number_of_some_items_per_page = 5;

  //
  // inject items at random places
  $vars['testimonials'] = array();
  _randomly_injected_testimonials($vars, $number_of_testimonials_per_page);
  _randomly_injected_testimonials($vars, $number_of_specials_per_page, true);
  _randomly_injected_social_items($vars, $number_of_some_items_per_page);
}

/**
 * Creates an associative array with randomly chosen testimonials to be added at random places in the view.
 */
function _randomly_injected_testimonials(&$vars, $testimonials_per_page, $only_special_items = false) {
  //
  // get the testimonials
  $linked_testimonials = _get_linked_testimonials($only_special_items);

  //
  // inject linked testimonials at random places
  $pager = $vars['view']->query->pager;
  $injected_testimonials = array();
  _define_injected_items_of_page($pager, $linked_testimonials, $testimonials_per_page, $injected_testimonials);
  $vars['testimonials'] = array_merge($vars['testimonials'], $injected_testimonials);
}

/**
 * Gets the testimonials from the DB.
 */
function _get_linked_testimonials($only_special_items = false) {
  global $language;
  $testimonials = array();

  //
  // get the published testimonials in given language
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'testimonial')
    ->propertyCondition('status', NODE_PUBLISHED)
    ->fieldCondition('field_explains_category', 'value', $only_special_items ? 1 : 0)
    ->propertyCondition('language', $language->language);

  $result = $query->execute();
  if ($result && !empty($result['node'])) {
    $testimonials = node_load_multiple(array_keys($result['node']));
  }

  //
  // render the testimonials and add the link (language dependent)
  switch ($language->language) {
    case 'de':
    default:
      $default_link = 'http://www.gruenewirtschaft.ch';
      break;
    case 'fr':
      $default_link = 'http://www.economieverte.ch';
      break;
  }
  $linked_testimonials = array();
  foreach ($testimonials as $key => $testimonial) {
    $testimonial_link = !empty($testimonial->field_link) ? $testimonial->field_link[LANGUAGE_NONE][0]['url'] : $default_link;
    $testimonial_teaser = node_view($testimonial, 'teaser');
    $linked_testimonials[] = array (
      'rendered_testimonial' => render($testimonial_teaser),
      'testimonial_link' => $testimonial_link,
    );
  }
  return $linked_testimonials;
}

/**
 * Picks a count of SOME items for each post view page and inject it at random places.
 */
function _randomly_injected_social_items(&$vars, $social_items_per_page) {
  //
  // get the SOME items of last cron run and cache them as long as the next cron run occurs
  $session_data = &pixelgarage_session_data();
  $social_items = array();
  $last_cron_run = variable_get('esgeht_cron_last_run', 0);

  if (isset($session_data['social_timestamp']) && ($session_data['social_timestamp'] > $last_cron_run) &&
      isset($session_data['$social_items'])) {
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
    $session_data['social_timestamp'] = REQUEST_TIME;
  }

  //
  // Inject a number of SOME items into each post page atbrandom places
  //
  // get injected items for current post page
  $pager = $vars['view']->query->pager;
  $injected_items = array();
  $end_index = _define_injected_items_of_page($pager, $social_items, $social_items_per_page, $injected_items);

  $vars['social_items'] = $injected_items;
  $vars['social_items_rest'] = array();

  // last page, add 20 items of the rest of SOME items at the end of the list
  if ($end_index && count($social_items) > $end_index) {
    $vars['social_items_rest'] = array_slice($social_items, $end_index, 20);
  }

}

/**
 * Returns an array of injectable items in the parameter. The keys of the returned array are the randomly
 * chosen indices of the post view rows, where the items have to be injected.
 *
 * @param $pager  object
 *    Post view pager
 * @param $items  array
 *    Items to be injected
 * @param $items_per_page int
 *    Number of injected items per post page
 * @return bool
 *    Returns index of the first unused item in the input items array.
 */
function _define_injected_items_of_page($pager, $items, $items_per_page, &$injected_items) {
  // set post view page information
  $total_posts = $pager->total_items;
  $current_page = $pager->current_page;
  $posts_per_page = (int)$pager->options['items_per_page'];
  $is_last_page = $total_posts < (($current_page + 1) * $posts_per_page);

  // define injected items for current page at random places
  $total_items = count($items);
  $start_index = $current_page * $items_per_page;
  // restart in items list when list end has reached
  $start_index = ($start_index < $total_items) ? $start_index : 0;
  $end_index =  ($current_page + 1) * $items_per_page;

  // get injected items for current page at random list places
  $keys = _distinct_random_numbers(0, $posts_per_page-1, $items_per_page);
  foreach ($keys as $index => $key) {
    if ($total_items > ($start_index + $index)) {
      $injected_items[$key] = $items[$start_index + $index];
    }
  }

  return $is_last_page ? $end_index : false;
}

/**
 * Returns an array of $count distinct random numbers between $min and $max.
 * If the range of distinct random numbers is smaller than the count of returned numbers,
 * all numbers between min and max are returned.
 */
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
