(function ($) {
  $(document).ready(function() {
    $('.conversion-rates input.conversion-rate').keyup(function(){
      var rate = $(this).val();
      if ($(this).val().search(',')) {
        $(this).val($(this).val().replace(',', '.'));
      }
      var example = '--';
      if (Number(rate)) {
        example = rate * $('#edit-demo-amount').val();
      }
      $(this).parent().find('.demo-amount-converted').html(example);
    });
    $('#edit-demo-amount').keyup(function(){
      if ($(this).val().search(',')) {
        $(this).val($(this).val().replace(',', '.'));
      }
      if (!Number($(this).val())) {
        $(this).val(100);
      }
      $('.demo-amount').html($(this).val());
      $('.conversion-rates input.conversion-rate').trigger('keyup');
    });
  });
})(jQuery);
