<?php
/**
 * @file
 * Bootstrap 12 template for Display Suite.
 */
//
// set language dependent heading
$content = $variables['content'];
$post_title = t('Support campaign!');
$post_heading = t('Share this post with your friends and family members!');
$post_claim = t('#EsGeht');
?>


<?php if (!$teaser): ?>
<div class="node-post-page-wrapper">
    <div class="node-post-header">
      <div class="post-title">
        <?php print $post_title ?>
      </div>
      <div class="post-heading">
        <?php print $post_heading ?>
      </div>
      <div class="social-share">
        <!-- AddtoAny social share buttons -->
        <div class="a2a_kit a2a_kit_size_32 a2a_default_style">
          <a class="a2a_button_facebook"></a>
          <a class="a2a_button_twitter"></a>
          <a class="a2a_button_google_plus"></a>
        </div>
        <script async src="https://static.addtoany.com/menu/page.js"></script>
      </div>
    </div>
    <div class="node-post-wrapper">
<?php endif; ?>

<<?php print $layout_wrapper; print $layout_attributes; ?> class="<?php print $classes; ?>">
  <?php if (isset($title_suffix['contextual_links'])): ?>
    <?php print render($title_suffix['contextual_links']); ?>
  <?php endif; ?>
  <div class="row">
    <<?php print $central_wrapper; ?> class="col-sm-12 <?php print $central_classes; ?>">
      <?php print render($content['field_image']); ?>
      <?php if (!empty($post_claim)): ?>
        <div class="quote-wrapper">
          <?php print render($content['field_quote']); ?>
          <span class="post-claim"><?php print $post_claim; ?></span>
        </div>
      <?php else: ?>
        <?php print render($content['field_quote']); ?>
      <?php endif; ?>
      <?php print render($content['field_your_name']); ?>
      <?php print render($content['group_icons']); ?>
    </<?php print $central_wrapper; ?>>
  </div>
</<?php print $layout_wrapper ?>>

<?php if (!$teaser): ?>
  <!-- close node wrapper and node-page-wrapper-->
  </div>
</div>
<?php endif; ?>



<!-- Needed to activate display suite support on forms -->
<?php if (!empty($drupal_render_children)): ?>
  <?php print $drupal_render_children ?>
<?php endif; ?>
