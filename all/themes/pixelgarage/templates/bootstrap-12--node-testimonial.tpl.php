<?php
/**
 * @file
 * Bootstrap 12 template for Display Suite.
 */
//
// set language dependent heading
$testimonial_claim = t('Ja zur Grünen Wirtschaft am 25. 9. 2016');
?>


<<?php print $layout_wrapper; print $layout_attributes; ?> class="<?php print $classes; ?>">
  <?php if (isset($title_suffix['contextual_links'])): ?>
    <?php print render($title_suffix['contextual_links']); ?>
  <?php endif; ?>
  <div class="row">
    <<?php print $central_wrapper; ?> class="col-sm-12 <?php print $central_classes; ?>">
      <?php print $central; ?>
      <div class="campaign-claim"><?php print $testimonial_claim; ?></div>
    </<?php print $central_wrapper; ?>>
  </div>
</<?php print $layout_wrapper ?>>

<!-- Needed to activate display suite support on forms -->
<?php if (!empty($drupal_render_children)): ?>
  <?php print $drupal_render_children ?>
<?php endif; ?>
