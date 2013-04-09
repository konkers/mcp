<?
  -- Lua server pages have full control over the output, including HTTP
  -- headers they send to the client. Send HTTP headers:
  print('HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n')

print_float = function(f)
	print(string.format("%.1f",f))
end

function add_gauge(name, temp)
	print(string.format("<div id=\"gauge_%s\" class=\"mcp_gauge\"></div>\n", name));
end

function add_switch(name, value)
  print(string.format("<div id=\"switch_%s\" class=\"mcp_switch\">%d</div>\n", name, value));
end

?>
<html>
  <head>
    <script src="jquery.js"></script>
    <script src="raphael.js"></script>
    <script src="justgage.js"></script>
    <script src="jquery.bpopup.js"></script>
    <script src="mcpgauge.js"></script>
    <script src="mcpswitch.js"></script>
    <script type='text/javascript'>

    var gauges = {};
    var outputs = {};

    $(document).ready(function() {
    	loadState();
    });

    function updateState() {
    	$.ajax({
    		url: "state.lp",
    		type: "GET",
    		dataType: "json",
    		success: function(data) {
    			$.each(data.sensors, function(key, sensor) {
            gauges[sensor.name].update(sensor);
    			});
          $.each(data.outputs, function(key, output) {
            outputs[output.name].update(output);
          });
    			setTimeout(function() { updateState(); }, 1000);
    		},

    		error: function( xhr, status ) {
    		},
    	});
    }

    function loadState() {
    	$.ajax({
    		url: "state.lp",
    		type: "GET",
    		dataType: "json",
    		success: function(data) {
    			$.each(data.sensors, function(key, sensor) {
            gauges[sensor.name] = new MCPGauge(sensor);
    			});
          $.each(data.outputs, function(key, output) {
            outputs[output.name] = new MCPSwitch(output);
          });
    			setTimeout(function() { updateState(); }, 1000);
    		},

    		error: function( xhr, status ) {
    		},
    	});
    }

    </script>
    <style>
    .mcp_gauge {
      float: left;
      width: 250px;
      height: 200px;

    }
    .gauges {
      clear:both;
    }
    .switches {
      clear:both;
    }
    .pid {
      background: #ffffff;
    }
    </style>
  </head>
  <body>
    <div id="gauges" class="gauges">
      <?
      for i, sensor in ipairs(sensors) do
        add_gauge(sensor["name"], sensor["temp"]);
      end ?>
    </div>
    <div id="switches" class="switches">
      <?
      for i, output in ipairs(outputs) do
        add_switch(output["name"], output["value"]);
      end ?>
    </div>
    <div>
      <pre><? print(err_string) ?></pre>
  </div>
	<div id="debug"></div>
  </body>
</html>
