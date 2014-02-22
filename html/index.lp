<?
  -- Lua server pages have full control over the output, including HTTP
  -- headers they send to the client. Send HTTP headers:
  print('HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n')


function find_sensor(sensors, name)
  for i, sensor in ipairs(sensors) do
    if sensor["name"] == name then
        return sensor
    end
  end
end

function find_output(outputs, name)
  for i, output in ipairs(outputs) do
    if output["name"] == name then
        return output
    end
  end
        add_switch(output["name"], output["value"]);
end

print_float = function(f)
	print(string.format("%.1f",f))
end

function add_gauge_name(sensors, name)
        sensor = find_sensor(sensors, name)
        add_gauge(sensor["name"], sensor["temp"])
end
function add_gauge(name, temp)
	print(string.format("<div id=\"gauge_%s\" class=\"mcp_gauge\"></div>\n", name));
end

function add_switch_name(outputs, name)
        output = find_sensor(outputs, name)
        add_switch(output["name"], output["temp"])
end
function add_switch(name, value)
  print(string.format("<div id=\"switch_%s\" class=\"mcp_switch\"></div>\n", name));
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
                            if (sensor.name in gauges) {
                                gauges[sensor.name].update(sensor);
                            }
    			});
                        $.each(data.outputs, function(key, output) {
                            if (output.name in outputs) {
                               outputs[output.name].update(output);
                            }
                        });
                        $("#update_time").html(data.pid_last_update);
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
                            if ($("#gauge_" + sensor.name).length) {
                                gauges[sensor.name] = new MCPGauge(sensor);
                            }
    			});
                        $.each(data.outputs, function(key, output) {
                            if ($("#switch_" + output.name).length) {
            outputs[output.name] = new MCPSwitch(output);
            }
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
      padding-left: 50px;
      padding-right: 50px;
    }
    .mcp_switch {
      float: left;
      width: 200px;
      height: 200px;
      padding: 50px;
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
    <center>
      <div id="time">Time since update: <div id="update_time">hi</div></div>
      <div id="gauges" class="gauges">
        <?
        add_gauge_name(sensors, "hlt")
        add_gauge_name(sensors, "RIMS")
        add_gauge_name(sensors, "mlt")
        add_gauge_name(sensors, "boil")
      ?>
    </div>
    <div id="switches" class="switches">
      <?
        add_switch_name(outputs, "hlt")
        add_switch_name(outputs, "mlt")
      ?>
    </div>
  </center>
    <div>
      <pre><? print(err_string) ?></pre>
  </div>
	<div id="debug"></div>
  </body>
</html>
