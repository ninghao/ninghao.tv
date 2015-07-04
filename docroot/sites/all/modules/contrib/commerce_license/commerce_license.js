(function ($) {
  Drupal.behaviors.commerceLicenseCheckout = {
    attach: function(context, settings) {
      $('.commerce-license-checkout').once('ajax-processed').each(function() {
        if (typeof $(this).data('refresh-url') != 'undefined') {
          var base = $(this).attr('id');
          var element_settings = {
            'url': $(this).data('refresh-url'),
            'type': 'throbber',
            'event': 'license-refresh-event'
          };
          Drupal.ajax[base] = new Drupal.ajax(base, this, element_settings);
        }
      });
      // We want this to be executed each time the behaviors are reattached.
      $('.commerce-license-checkout').each(function() {
        if (typeof $(this).data('refresh-rate') != 'undefined') {
          // Convert the refresh rate into milliseconds.
          var refreshRate = $(this).data('refresh-rate') * 1000;
          var elm = $(this);
          if (refreshRate != 0) {
            setTimeout(function() {
              elm.trigger('license-refresh-event');
            },
              refreshRate
            );
          }
        }
      });
    }
  }
})(jQuery);
