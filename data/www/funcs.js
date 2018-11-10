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
  $('#save').on('click', function() {
    $(this).prop('disabled', 'disabled');
    $(this).text('Saving...');
    user = $('#user').val();
    pass = $('#pass').val();
    ssid = $('#ssid').val();
    if(enc) {
      psk = $('#psk').val();
      $.ajax({
        type: 'POST',
        url: '/setup',
        contentType: 'application/json',
        data: JSON.stringify({system: {username: user, password: pass}, wifi: {ssid: ssid, psk: psk}}),
      }).done(function() {
        $('#save').text('Save');
        $('#save').removeAttr('disabled');
        $('#done').modal('show');
      });
    } else {
      $.ajax({
        type: 'POST',
        url: '/setup',
        contentType: 'application/json',
        data: JSON.stringify({system: {username: user, password: pass}, wifi: {ssid: ssid}}),
      }).done(function() {
        $('#save').text('Save');
        $('#save').removeAttr('disabled');
        $('#done').modal('show');
      });
    }
  });
};
