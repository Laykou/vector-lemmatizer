$(function() {
	$('form#input').submit(function (e) {
		e.preventDefault();

		var $button = $(this).find('button');
		$button.text('Please wait...').prepend($('<i/>').addClass('fa fa-fw fa-circle-o-notch fa-spin')).prop('disabled', true);

		var unlock = function() {
			$button.text('Submit').prop('disabled', false);
		};

		$.ajax({
			url: 'api/',
			type: 'POST',
			data: $(this).serializeArray(),
			dataType: 'json',
			success: function (data) {
				unlock();
				try {
					var results = "";
					$.each(data.results, function(index, value) {
						results += value + "\n";
					});
					$('#output').val(results);
				} catch (e) {
					$('#output').val("Error while processing request");
				}
				$('#output').trigger("change");
			},
			error: function (xhr) {
				unlock();
				try {
					data = $.parseJSON(xhr.responseText);
					alert(data.message);
				} catch (e) {
					$('#output').val("Error while processing request");
					$('#output').trigger("change");
				}
			}
		});
	});

	$('#output').change(function() {
		var $table = $("#output-table");
		$table.html("");
		var lines = $("#output").val().split("\n");
		for (i=0;i<lines.length;++i) {
			if (lines[i] != '') {
				var $tr = $("<tr>");
				var cells = lines[i].split(/\s/);
				for (j=0;j<cells.length;++j) {
					$tr.append($("<td>").text(cells[j]));
				}
				$table.append($tr);
			}
		}
	});
});