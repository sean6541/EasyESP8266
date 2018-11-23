window.onload = function() {
  enc = true;
  $('#haspsk').on('change', function() {
    if($(this).is(':checked')) {
      enc = false;
      $('#pskc').hide();
    } else {
      enc = true;
      $('#pskc').show();
    }
  });
  $('#connect').on('click', function() {
    $(this).prop('disabled', 'disabled');
    $(this).text('Connecting...');
    ssid = $('#ssid').val();
    if(enc) {
      psk = $('#psk').val();
      $.ajax({
        type: 'POST',
        url: '/connect',
        contentType: 'application/json',
        data: JSON.stringify({ssid: ssid, psk: psk}),
      }).done(function() {
        $('#connect').text('Connect');
        $('#connect').removeAttr('disabled');
        $('#done').modal('show');
      });
    } else {
      $.ajax({
        type: 'POST',
        url: '/connect',
        contentType: 'application/json',
        data: JSON.stringify({ssid: ssid}),
      }).done(function() {
        $('#connect').text('Connect');
        $('#connect').removeAttr('disabled');
        $('#done').modal('show');
      });
    }
  });
};
