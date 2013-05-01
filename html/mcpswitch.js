MCPSwitch = function(output) {
	this.name = output.name;
	this.element = $('#switch_' + this.name);
    this.updating = false;
    var h = this.element.height();
    var w = this.element.width();
    console.log("w=" + w + " h=" + h);
    var paper = Raphael('switch_' + this.name, w, h);
    this.paper = paper

    paper.clear();

//    paper.rect(0, 0, w, h);

    var knob = this.paper.set()
    this.knob = knob;
    var title_txt_attr = {
        "font-weight": "bold",
        "font-family": "Arial",
        "font-size": "32px"};

    var label_txt_attr = {
        "font-weight": "bold",
        "font-family": "Arial",
        "font-size": "18px"};

    knob.push(
        paper.circle(0, 0, 50).attr({"fill": "r#fff:75-#888", "stroke-width": 0}),
        paper.circle(0, -30, 7).attr({"fill": "r#fff:40-#888", "stroke-width": 0})
        );

    var labels = this.paper.set();
    this.labels = labels;
    labels.push(
        paper.text(0, -100, this.name).attr(title_txt_attr),
        paper.text(-90, -60, "recirc").attr(label_txt_attr).attr({"text-anchor": "start"}),
        paper.text(90, -60, "transfer").attr(label_txt_attr).attr({"text-anchor": "end"})
        );


    var minDim = (h > w) ? w : h;
    var scale = (minDim / 200.0).toFixed(1);
    this.transformation = "S" + scale + "," + scale +" 0,0 T" + (w / 2) + "," + (h * 0.6);
    console.log(this.transformation);

    if (output.value == 0)
        knob.transform("r-45 0,0 " + this.transformation);
    else
        knob.transform("r45 0,0 " + this.transformation);

    labels.transform(this.transformation);
    this.element.bind('click', this, function(e) {
        e.preventDefault();
        e.data.clicked();
    });
//    this.element.html(

//        '<div class="mcp_switch_name">' + this.name + '</div>' +
//        '<input type="radio" class="mcp_switch_input" id="switch_flow_' + this.name + '_0" ' +
//        'name="switch_flow_' + this.name + '" value="0">Recirculate</input>' +
//        '<input type="radio" class="mcp_switch_input" id="switch_flow_' + this.name + '_1" ' +
//       'name="switch_flow_' + this.name + '" value="1">Transfer</input>');
//    $("input[name='switch_flow_" + this.name + "']").change(
//        this, function(e) {
//            e.preventDefault();
//            e.data.valueChanged();
//        });

//    this.update(output);
}

MCPSwitch.prototype.update = function(output) {
//    if (!this.updating) {
//        $('#switch_flow_' + this.name + '_0').prop('checked', output.value == 0);
//        $('#switch_flow_' + this.name + '_1').prop('checked', output.value == 1);
//    }
};

// below are fucntions that should be private if javascript supported that
MCPSwitch.prototype.clicked = function() {
    this.updating = true;
    this.value = this.value ? 0 : 1;
    var postData = {
        'name': this.name,
        'value': this.value,
    };
    var sw = this;
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

    if (this.value == 0)
        this.knob.animate({"transform": "r-45 0,0 " + this.transformation}, 200);
    else
        this.knob.animate({"transform": "r45 0,0 " + this.transformation}, 200);
}