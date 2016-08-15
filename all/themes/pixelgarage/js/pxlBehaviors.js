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
   * Allows full size clickable cards (open node in full size).
   *
   * Remark: Disable this behavior, if flip card are used
   */
  Drupal.behaviors.fullSizeClickableCards = {
    attach: function () {
      var $clickableItems = $('.view-post-grid .pe-item-no-ajax'),
          $linkedItems = $('.view-post-grid .pe-item-linked');

      $clickableItems.once('click', function () {
        $(this).on('click', function () {
          window.location = $(this).find(".node-post .field-name-field-image a").attr("href");
          return false;
        });
      });
      $linkedItems.once('click', function () {
        $(this).on('click', function () {
          var link = $(this).find(".pe-item-inner > a").attr("href");
          window.open(link, '_blank');
          return false;
        });
      });
    }
  };

  Drupal.behaviors.createToggleFilters = {
    attach: function() {
      var $categoryWrapper = $('#edit-field-category-tid-wrapper'),
          $categoryFilters = $categoryWrapper.find('.form-type-bef-link'),
          $activeFilter = $categoryFilters.find('>a.active'),
          $categoryReset = $categoryWrapper.find('.form-item-edit-field-category-tid-all'),
          categoryResetLink = $categoryReset.find('>a').attr('href');

      //
      // hide reset filter and make active filter a toggle filter
      $categoryReset.hide();

      if ($activeFilter) {
        $activeFilter.attr('href', categoryResetLink);
      }
    }
  };

  Drupal.behaviors.createToggleSort = {
    attach: function() {
      var $sortWrapper = $('.views-widget-sort-by'),
          $sortItems = $sortWrapper.find('.form-type-bef-link'),
          $activeSort = $sortItems.find('>a.active'),
          $sortReset = $sortWrapper.find('.form-item-edit-sort-by-created'),
          sortResetLink = $sortReset.find('>a').attr('href');

      //
      // hide reset filter and make active filter a toggle filter
      $sortReset.hide();

      if ($activeSort) {
        $activeSort.attr('href', sortResetLink);
      }
    }
  };


})(jQuery);
