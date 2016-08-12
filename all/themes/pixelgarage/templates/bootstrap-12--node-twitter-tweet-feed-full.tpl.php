<?php
/**
 * @file
 * Bootstrap 12 template for Display Suite.
 */

$screen_name = $field_tweet_author_name[0]['value'];
$tweet_id = (int)$field_tweet_id[LANGUAGE_NONE][0]['value'];
$tweet_url = $field_link_to_tweet[0]['value'];


$reply = t('Reply');
$retweet = t('Retweet');
$favorite = t('Favorite');
$goto_tweet = t('Go to tweet');

?>


<<?php print $layout_wrapper; print $layout_attributes; ?> class="<?php print $classes; ?>">
  <?php if (isset($title_suffix['contextual_links'])): ?>
    <?php print render($title_suffix['contextual_links']); ?>
  <?php endif; ?>
  <div class="row">
    <<?php print $central_wrapper; ?> class="col-sm-12 <?php print $central_classes; ?>">

      <!-- Print fields -->
      <?php print render($content['field_profile_image']); ?>
      <?php print render($content['field_tweet_author']); ?>
      <?php print render($content['field_tweet_author_name']); ?>

      <!-- Print tweet text -->
      <?php print render($content['field_tweet_contents']); ?>

      <!-- Print tweet date -->
      <?php print render($content['field_tweet_creation_date']); ?>

      <ul class="actions">
        <li><a href="<?php print $tweet_url; ?>" target="_blank"><?php print $goto_tweet; ?></a></li>
        <li><a href="https://twitter.com/intent/tweet?in_reply_to=<?php print $tweet_id; ?>" target="_blank"><?php print $reply; ?></a></li>
        <li><a href="https://twitter.com/intent/retweet?tweet_id=<?php print $tweet_id; ?>" target="_blank"><?php print $retweet; ?></a></li>
        <li><a href="https://twitter.com/intent/favorite?tweet_id=<?php print $tweet_id; ?>" target="_blank"><?php print $favorite; ?></a></li>
      </ul>

    </<?php print $central_wrapper; ?>>
  </div>
</<?php print $layout_wrapper ?>>


<!-- Needed to activate display suite support on forms -->
<?php if (!empty($drupal_render_children)): ?>
  <?php print $drupal_render_children ?>
<?php endif; ?>
