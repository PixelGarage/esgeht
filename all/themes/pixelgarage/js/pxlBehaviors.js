/**
 * This file contains all Drupal behaviours of the pixelgarage theme.
 *
 * Created by ralph on 05.01.14.
 */

(function ($) {

  /**
   * This behavior adds shadow to header on scroll.
   *
   Drupal.behaviors.addHeaderShadow = {
    attach: function (context) {
      var isFixedHeader = true;

      $(window).off("scroll");
      $(window).on("scroll", function () {
        var $header              = $("header.navbar"),
            $headerCont          = $("header.navbar .container"),
            fixedHeaderScrollPos = 135,
            $width               = $(window).width();

        if ($width >= 1024) {
          fixedHeaderScrollPos = 135;
        }
        else if ($width >= 768) {
          fixedHeaderScrollPos = 95;
        }
        else if ($width >= 480) {
          fixedHeaderScrollPos = 72;
        }
        else {
          fixedHeaderScrollPos = 60;
        }

        if ($(window).scrollTop() >= fixedHeaderScrollPos) {
          if (isFixedHeader) return;

          // keep header fixed at this scroll position
          $header.removeClass('navbar-static-top').addClass('navbar-fixed-top');
          $('body').removeClass('navbar-is-static-top').addClass('navbar-is-fixed-top');

          // fix header and add shadow
          $header.css({position: 'fixed', top: -fixedHeaderScrollPos + 'px'});
          $headerCont.css("box-shadow", "0 4px 3px -4px gray");

          isFixedHeader = true;
        }
        else {
          if (!isFixedHeader) return;

          // set header to static in top scroll region
          $header.removeClass('navbar-fixed-top').addClass('navbar-static-top');
          $('body').removeClass('navbar-is-fixed-top').addClass('navbar-is-static-top');

          // remove shadow from header
          $header.css({position: 'static', top: 'auto'});
          $headerCont.css("box-shadow", "none");

          isFixedHeader = false;
        }
      });
    }
  };
   */

  /**
   * Set a class defining the device, e.g. mobile-device or desktop.
   */
  Drupal.behaviors.setMobileClass = {
    attach: function (context) {
      if (isMobile.any) {
        $('body').addClass('mobile-device');
      }
      else {
        $('body').addClass('desktop');
      }
    }
  };

  Drupal.behaviors.shariffFrontuttons = {
    attach: function(context) {
      var $window = $(window),
          $shariffContainer = $('.main-container .shariff ul');

      $shariffContainer.once('resize', function() {
        $window.on('resize', function(ev) {
          if ($window.width() >= 768) {
            $shariffContainer.removeClass('orientation-horizontally').addClass('orientation-vertical')
          }
          else {
            $shariffContainer.removeClass('orientation-vertical').addClass('orientation-horizontally')
          }
        });

        $(this).resize();
      });

    }
  };


  /**
   * Implements the reset behavior of all buttons, e.g. de/activation of reset button
   * according to the sort and filter button states.
   *
   * @type {{attach: Drupal.behaviors.activateResetButton.attach}}
   */
  Drupal.behaviors.activateResetButton = {
    attach: function () {
      var $toggleFilterWrapper = $('#toggle-filters-wrapper'),
        $categoryWrapper = $toggleFilterWrapper.find('#edit-field-category-tid-wrapper'),
        $sortWrapper = $toggleFilterWrapper.find('.views-widget-sort-by'),
        $resetCategoryLink = $categoryWrapper.find('.form-item-edit-field-category-tid-all > a'),
        $resetSortLink = $sortWrapper.find('.form-item-edit-sort-by-created > a'),
        $resetButton = $toggleFilterWrapper.find('.views-reset-button > button');

      //
      // de/activate reset button
      if ($resetCategoryLink.hasClass('active') && $resetSortLink.hasClass('active')) {
        $resetButton.addClass('active');
      }
    }
  };


})(jQuery);
