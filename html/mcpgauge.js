
MCPGauge = function(sensor) {
	this.name = sensor.name;
	this.element = $('#gauge_' + sensor.name);
	this.element.html('<div class="gage" id="gage_' + sensor.name + '"></div>');
	this.gage = new JustGage({
    		id: "gage_" + sensor.name,
    		title: sensor.name,
    		value: sensor.temp,
    		symbol: 'C',
    		decimals: 1,
    		min: 0,
    		max: 100,
    		startAnimationTime: 10,
    		refreshAnimationTime: 10,
    		customSectors: [{
    			color : "#ff0000",
    			lo : 0,
    			hi : 61
    		},{
    			color : "#ffff00",
    			lo : 61,
    			hi : 64.5
    		}, {
    			color : "#00ff00",
    			lo : 64.5,
    			hi : 65.5
     		},{
    			color : "#ffff00",
    			lo : 65.5,
    			hi : 69.0
    		}, {
    			color : "#ff0000",
    			lo : 69.0,
    			hi : 100
    		}]
    	});

    	if ('pid' in sensor) {
    		this.element.append('<div class="pid" id="pid_' + sensor.name + '" style="display:none"></div>');
    		this.pid_settings = $('#pid_' + sensor.name);
    		this.pid_settings.html(
    			'<table border="1">' +
    			'<tr><th>Set Point:</th><td><input type="text" id="pid_' + sensor.name + '_set_point" /></td></tr>' +
    			'<tr><th>P Term:</th><td><input type="text" id="pid_' + sensor.name + '_p" /></td></tr>' +
    			'<tr><th>I Term:</th><td><input type="text" id="pid_' + sensor.name + '_i" /></td></tr>' +
    			'<tr><th>D Term:</th><td><input type="text" id="pid_' + sensor.name + '_d" /></td></tr>' +
    			'<tr><th>pkt_1:</th><td id="pid_' + sensor.name + '_pkt_1"></td></tr>' +
    			'<tr><th>ekt_1:</th><td id="pid_' + sensor.name + '_ekt_1"></td></tr>' +
    			'</table>' +
    			'<input type="submit" id="pid_' + sensor.name + '_update" value="Update"/>');

    		$('#pid_' + sensor.name + '_update').bind('click', this, function(e) {
    			e.data.updatePidSettings();
    		});

    		this.pid = {
    			'set_point': $('#pid_' + sensor.name + '_set_point'),
    			'p': $('#pid_' + sensor.name + '_p'),
    			'i': $('#pid_' + sensor.name + '_i'),
    			'd': $('#pid_' + sensor.name + '_d'),
    			'pkt_1': $('#pid_' + sensor.name + '_pkt_1'),
    			'ekt_1': $('#pid_' + sensor.name + '_ekt_1')
    		};
    		this.setPid(sensor);

    		this.element.bind('click', this, function(e) {
    			e.preventDefault();
    			e.data.pid_settings.bPopup();
    		});
    	}
}

MCPGauge.prototype.update = function(sensor) {
	this.gage.refresh(sensor.temp);
	if ('pid' in sensor) {
		this.updatePid(sensor);
	}
};

// below are fucntions that should be private if javascript supported that
MCPGauge.prototype.updatePid = function(sensor) {
	this.pid['pkt_1'].text(sensor.pid.pkt_1.toFixed(2));
	this.pid['ekt_1'].text(sensor.pid.ekt_1.toFixed(2));
};

MCPGauge.prototype.setPid = function(sensor) {
	this.pid['set_point'].val(sensor.pid.set_point.toFixed(1));
	this.pid['p'].val(sensor.pid.p.toFixed(2));
	this.pid['i'].val(sensor.pid.i.toFixed(2));
	this.pid['d'].val(sensor.pid.d.toFixed(2));
	this.updatePid(sensor);
};

MCPGauge.prototype.updatePidSettings = function() {
	var postData = {
		'name': this.name,
		'set_point': this.pid['set_point'].val(),
		'p': this.pid['p'].val(),
		'i': this.pid['i'].val(),
		'd': this.pid['d'].val(),
	};
	var gauge = this;
	$.ajax({
    		url: "/pid_update",
    		type: "POST",
    		data: postData,
    		dataType: "json",
    		success: function(data) {
    			gauge.pid_settings.close();
    		},

    		error: function( xhr, status ) {
    			console.log("error " + status);
    			console.log(xhr);
    		},
    	});
}

MCPGauge.prototype.stuff = function() {
	console.log("stuff");
}