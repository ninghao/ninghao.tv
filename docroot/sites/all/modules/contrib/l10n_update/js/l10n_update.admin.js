(function ($) {

/**
 * Show/hide the description details on Translate interface update page.
 */
Drupal.behaviors.hideUpdateInformation = {
  attach: function (context, settings) {
    var $table = $('#l10n-update-status-form').once('expand-updates');
    if ($table.length) {
      var $tbodies = $table.find('tbody');

      // Open/close the description details by toggling a tr class.
      $tbodies.find('.description').bind('click keydown', function (e) {
        if (e.keyCode && (e.keyCode !== 13 && e.keyCode !== 32)) {
          return;
        }
        e.preventDefault();
        var $tr = $(this).closest('tr');

        $tr.toggleClass('expanded');

        // Change screen reader text.
        $tr.find('.update-description-prefix').text(function () {
          if ($tr.hasClass('expanded')) {
            return Drupal.t('Hide description');
          }
          else {
            return Drupal.t('Show description');
          }
        });
      });
      $table.find('.requirements, .links').hide();
    }
  }
};

})(jQuery);
