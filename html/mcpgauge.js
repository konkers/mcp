
MCPGauge = function(sensor) {
	this.name = sensor.name;
	this.element = $('#gauge_' + sensor.name);
	this.element.html('<div class="gage" id="gage_' + sensor.name + '"></div>');

    if ('pid' in sensor) {
        customSectors = this.genSectors(sensor.pid.set_point, 3);
    } else {
        customSectors = [];
    }
    console.log(sensor);
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
            levelColors: ["#0000ff", "#0000ff", "#0000ff"],
    		customSectors: customSectors
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
                '<tr><th>over_cycles:</th><td id="pid_' + sensor.name + '_over_cycles"></td></tr>' +
                '<tr><th>under_cycles:</th><td id="pid_' + sensor.name + '_under_cycles"></td></tr>' +
                '<tr><th>total_cycles:</th><td id="pid_' + sensor.name + '_total_cycles"></td></tr>' +
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
                'ekt_1': $('#pid_' + sensor.name + '_ekt_1'),
                'under_cycles': $('#pid_' + sensor.name + '_under_cycles'),
                'over_cycles': $('#pid_' + sensor.name + '_over_cycles'),
                'total_cycles': $('#pid_' + sensor.name + '_total_cycles')
    		};
    		this.setPid(sensor);

    		this.element.bind('click', this, function(e) {
    			e.preventDefault();
    			e.data.pid_settings.bPopup();
    		});
    	}
}

MCPGauge.prototype.update = function(sensor) {
    if ('pid' in sensor) {
        this.gage.config.customSectors = this.genSectors(sensor.pid.set_point, 3);
        console.log(this.gage.config.customSectors);
    }
	this.gage.refresh(sensor.temp);
	if ('pid' in sensor) {
		this.updatePid(sensor);
	}
};

// below are fucntions that should be private if javascript supported that
MCPGauge.prototype.updatePid = function(sensor) {
    var overCycles = parseInt(sensor.pid.over_cycles);
    var underCycles = parseInt(sensor.pid.under_cycles);

	this.pid['pkt_1'].text(sensor.pid.pkt_1.toFixed(2));
    this.pid['ekt_1'].text(sensor.pid.ekt_1.toFixed(2));

    this.pid['over_cycles'].text(overCycles);
    this.pid['under_cycles'].text(underCycles);
    this.pid['total_cycles'].text(overCycles + underCycles);
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

MCPGauge.prototype.genSectors = function(setpoint, tolerance) {
    return [{
        color : "#ff0000",
        lo : 0,
        hi : setpoint - tolerance
    },{
        color : "#ffff00",
        lo : setpoint - tolerance,
        hi : setpoint - tolerance * 0.5
    }, {
        color : "#00ff00",
        lo : setpoint - tolerance * 0.5,
        hi : setpoint + tolerance * 0.5
    },{
        color : "#ffff00",
        lo : setpoint + tolerance * 0.5,
        hi : setpoint + tolerance
    }, {
        color : "#ff0000",
        lo : setpoint + tolerance,
        hi : 100
    }]

}