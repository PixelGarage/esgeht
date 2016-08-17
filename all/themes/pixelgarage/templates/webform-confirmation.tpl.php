<?php

/**
 * @file
 * Customize confirmation screen after successful submission.
 *
 * This file may be renamed "webform-confirmation-[nid].tpl.php" to target a
 * specific webform e-mail on your site. Or you can leave it
 * "webform-confirmation.tpl.php" to affect all webform confirmations on your
 * site.
 *
 * Available variables:
 * - $node: The node object for this webform.
 * - $progressbar: The progress bar 100% filled (if configured). This may not
 *   print out anything if a progress bar is not enabled for this node.
 * - $confirmation_message: The confirmation message input by the webform
 *   author.
 * - $sid: The unique submission ID of this submission.
 * - $url: The URL of the form (or for in-block confirmations, the same page).
 */

// Donate confirmation
$current_tnid = $node->tnid ? $node->tnid : $node->nid;
$delivery_tnid = 789;
$donation_tnid = 808;
$newsletter_tnid = 61;

$title = t('Thank you,');

switch ($current_tnid) {
  case $delivery_tnid:
    $message = t('for your investment into the green economy! We will send you the postcards in about a week!');
    break;
  case $donation_tnid:
    $message = t('for your investment into the green economy!');
    break;
  case $newsletter_tnid:
    $message = t('you have been successfully added to our newsletter.');
    break;
  default:
    $message = $confirmation_message;
    break;
}

$url = '/';
?>

<div class="webform-confirmation">
  <div class="confirmation-title"><?php print $title; ?></div>
  <div class="confirmation-message"><?php print $message; ?></div>
</div>

<div class="links">
  <a href="<?php print $url; ?>"><?php print t('Go back to home') ?></a>
</div>
