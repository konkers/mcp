MCPSwitch = function(output) {
	this.name = output.name;
	this.element = $('#switch_' + this.name);
    this.updating = false;
    this.element.html(
        '<div class="mcp_switch_name">' + this.name + '</div>' +
        '<input type="radio" class="mcp_switch_input" id="switch_flow_' + this.name + '_0" ' +
        'name="switch_flow_' + this.name + '" value="0">Recirculate</input>' +
        '<input type="radio" class="mcp_switch_input" id="switch_flow_' + this.name + '_1" ' +
        'name="switch_flow_' + this.name + '" value="1">Transfer</input>');
    $("input[name='switch_flow_" + this.name + "']").change(
        this, function(e) {
            e.preventDefault();
            e.data.valueChanged();
        });

    this.update(output);
}

MCPSwitch.prototype.update = function(output) {
    if (!this.updating) {
        $('#switch_flow_' + this.name + '_0').prop('checked', output.value == 0);
        $('#switch_flow_' + this.name + '_1').prop('checked', output.value == 1);
    }
};

// below are fucntions that should be private if javascript supported that
MCPSwitch.prototype.valueChanged = function() {
    var postData = {
        'name': this.name,
        'value': $("input[name='switch_flow_" + this.name + "']:checked").val(),
    };
    var sw = this;
    this.updating = true;
    $.ajax({
        url: "/output_update",
        type: "POST",
        data: postData,
        dataType: "json",
        success: function(data) {
            sw.updating = false;
        },

        error: function( xhr, status ) {
            console.log("error " + status);
            console.log(xhr);
        },
    });
}