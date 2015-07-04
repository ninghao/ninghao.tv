(function ($) {
  /**
    * Hide the submit button and submit the form as soon as the currency is
    * changed.
    */
  Drupal.behaviors.commerce_multicurrency = {
    attach: function (context, settings) {
      $('input#edit-save-selected-currency.form-submit').hide();
      $('#edit-selected-currency.form-select').change(function() {
        $(this).parent().closest('form').submit();
      });
    }
  };
})(jQuery);
