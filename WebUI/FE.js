/**
 * The FE class
 */


function FE(name_tmp, renderer, FEx) {
	this.renderer = renderer;
	this.name = name_tmp;
	this.form = new Array();
	this.minichart = new Array();
	this.FEx = FEx;
	this.FEy = 200;
	this.FEboxwidth = 150;
	this.FEboxhigh = 400;
	this.no_conn = 1;
	
	this.boxattr1 = {
			fill : {
				radialGradient: { cx: 0, cy: 0.3, r: 1 },
		 		stops: [ 
		 	         [0, 'white'], 
		 	         [1, '#BBBBBB'] ]
			},
			zIndex : 3,
//			opacity: 0
		};
	this.FEicon= 'fe_supermicro1.png';
	this.FEiconoffset=320;
	this.FEtxt_xoffset=60;
	this.FEtxt_yoffset=60;
	this.FE_iconwidth=150;
	this.FE_iconh=75;
	this.txtattr={
	        align: 'center',
	        zIndex: 5,
	        padding: 0,
	        opacity: 0
	    };
	this.txtcss={
	        fontSize: '50px',
	        color: "#577295"
	    }; 
	this.IE1boxy=125; 
	this.animate_Appear_attr = { opacity: 1 };
	this.animate_Disppear_attr = { opacity: 0 };
	this.animatetime={ duration: 1000 }; 
	this.lURL="index.html";
	
	this.gaugeh = 150;
	this.gaugew = 150;
	this.FEgauge_yoffset = 70;
	
	this.stopUpdates = true;
};

FE.prototype.requestDataGauge = function(gauge_data_url, gauge_index) {
//	console.log("ciao ", gauge_data_url);
	if(! this.stopUpdates) {
	var feObject = this;
    $.getJSON(
        gauge_data_url,
        function(data) {
        	if(gauge_index > 0) {
        	feObject.minichart[gauge_index].series[0].setData(data, true);
        	} else {
        		var point = feObject.minichart[gauge_index].series[0].points[0];
         		var newVal = data[1];
        		point.update(newVal);
        	}
        	// call it again after one second
          	setTimeout(function() {feObject.requestDataGauge(gauge_data_url, gauge_index);}, 3000);
        }
     );
	}
};

FE.prototype.DrawGaugeDiv = function() 
{
	var containerDiv = document.getElementById("container");
	var xoffset = containerDiv.offsetLeft;
	var yoffset = containerDiv.offsetTop;
	this.cont_g1 = document.createElement("div");
	this.cont_g1.id = "containerg1_" + this.name;
	this.cont_g1.style.position = "absolute";
	this.cont_g1.style.height = this.gaugeh + "px";
	this.cont_g1.style.width = this.gaugew + "px";
	this.cont_g1.style.left = xoffset+this.FEx+(this.FEboxwidth-this.gaugew)/2+"px";
	this.cont_g1.style.top = yoffset+this.FEy+this.FEgauge_yoffset+"px";
	document.body.appendChild(this.cont_g1);
	
	this.cont_g2 = document.createElement("div");
	this.cont_g2.id = "containerg2_" + this.name;
	this.cont_g2.style.position = "absolute";
	this.cont_g2.style.height = this.gaugeh/3 + "px";
	this.cont_g2.style.width = this.gaugew + "px";
	this.cont_g2.style.left = xoffset+this.FEx+(this.FEboxwidth-this.gaugew)/2+"px";
	this.cont_g2.style.top = yoffset+this.FEy+this.FEgauge_yoffset+this.gaugeh+"px";
	document.body.appendChild(this.cont_g2);
	
	this.cont_g3 = document.createElement("div");
	this.cont_g3.id = "containerg3_" + this.name;
	this.cont_g3.style.position = "absolute";
	this.cont_g3.style.height = this.gaugeh/3 + "px";
	this.cont_g3.style.width = this.gaugew + "px";
	this.cont_g3.style.left = xoffset+this.FEx+(this.FEboxwidth-this.gaugew)/2+"px";
	this.cont_g3.style.top = yoffset+this.FEy+this.FEgauge_yoffset+(4*this.gaugeh/3)+"px";
	document.body.appendChild(this.cont_g3);
	
};


/*
 * Form Drawing
 */

FE.prototype.draw = function()
{
	this.oldFEx=this.FEx;
	this.oldwidth=this.FEboxwidth;
	this.form.push( this.renderer.rect(this.FEx, this.FEy, this.FEboxwidth, this.FEboxhigh, 10)
			.attr(this.boxattr1)
			);
/*			.on('mouseover', function() {this.form[0].attr({fill :  'red'});})
			.on('mouseout', function() {this.form.attr(boxattr1)
										.attr({opacity : 1});
		}))*/;
	this.form.push( this.renderer.image(this.FEicon,this.FEx+(this.FEboxwidth-this.FE_iconwidth)/2, 
			this.FEy + this.FEiconoffset, this.FE_iconwidth, this.FE_iconh)
			.attr({	zIndex : 4 	}));
	this.form.push( this.renderer.text(this.name,this.FEx+this.FEboxwidth/2,this.FEy+this.FEtxt_yoffset)
			.attr(this.txtattr)
			.css(this.txtcss));
	for(var i=0; i< this.no_conn; i++) {
		this.form.push(this.renderer.path([ 
		                                    'M', this.FEx+this.FEboxwidth/2+10*(i-this.no_conn/2), this.FEy, 
		                                    'L', this.FEx+this.FEboxwidth/2+10*(i-this.no_conn/2), this.IE1boxy 
		                                    ])
				.attr({
					'stroke-width' : 5,
					stroke : 'white',
					opacity : 0
				}));
	}
	this.form.AddLink(this.lURL);
	
	this.form.animateRender(this.animate_Appear_attr, this.animatetime);
	this.form.addToRender();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	this.DrawGaugeDiv();
	this.stopUpdates = false;
	
	this.minichart.push(new Highcharts.Chart(get_gauge_option(this.cont_g1.id )));
	this.requestDataGauge(this.name+"_g0.json", 0);
	this.minichart.push(new Highcharts.Chart(get_hgauge_option(this.cont_g2.id , "Traffic Load")));
	this.requestDataGauge(this.name+"_g1.json", 1);
	this.minichart.push(new Highcharts.Chart(get_hgauge_option(this.cont_g3.id, "Power Consumption")));
	this.requestDataGauge(this.name+"_g2.json", 2);
	
	
};

FE.prototype.appear = function()
{
	this.stopUpdates = false;
	this.form.animateRender(this.animate_Appear_attr, this.animatetime);
	this.DrawGaugeDiv();
	this.minichart[0] = new Highcharts.Chart(get_gauge_option(this.cont_g1.id ));
	this.minichart[1] = new Highcharts.Chart(get_hgauge_option(this.cont_g2.id , "Traffic Load"));
	this.minichart[2] = new Highcharts.Chart(get_hgauge_option(this.cont_g3.id, "Power Consumption"));
	this.requestDataGauge(this.name+"_g0.json", 0);
	this.requestDataGauge(this.name+"_g1.json", 1);
	this.requestDataGauge(this.name+"_g2.json", 2);
};

FE.prototype.disappear = function()
{
	this.stopUpdates = true;
	this.form.animateRender(this.animate_Disppear_attr, this.animatetime);
	this.minichart[0].destroy();
	this.minichart[1].destroy();
	this.minichart[2].destroy();
	
	this.cont_g1.id.parentNode.removeChild(this.cont_g1.id);
	this.cont_g2.id.parentNode.removeChild(this.cont_g2.id);
	this.cont_g3.id.parentNode.removeChild(this.cont_g3.id);
};

FE.prototype.standby = function()
{
/*	this.form[0].attr(this.boxattr1);
	
	this.form[0].animate(
			{
				fill : 'gray',
				opacity: .5
			},this.animatetime);
	this.form[0].add();*/
	this.form[0].attr({fill: 'gray'});
	this.form[2].css({color: '#AAAAAA'});
	this.minichart[0].yAxis[0].update(
			{
				tickColor: '#FFFFFF',
				labels: {
		            step: 2,
		            rotation: 'auto',
		            style: {color : '#FFFFFF'}
		        },
				plotBands: [
					{
						from: 0, to: 100, 
						color:{
							linearGradient: { x1: 0, y1: 0, x2: 1, y2: 0},
							stops: [[0, '#999999'],[.5, '#BBBBBB'],[.6, '#BBBBBB'],[1,'#DDDDDD'] ]
							}
					}
				            /*{
					from: 0, to: 50, color: '#999999' // darker gray
				}, {
					from: 50, to: 80, color: '#BBBBBB' // middle gray
				}, {
					from: 80, to: 100, color: '#DDDDDD' // lighter gray
				}*/]
			}
	);
	this.minichart[1].yAxis[0].update(
			{
				labels: {
		            style: {color : '#FFFFFF'}
		        }
			});
	this.minichart[2].yAxis[0].update(
			{
				labels: {
		            style: {color : '#FFFFFF'}
		        }
			});
		
};

FE.prototype.wakeup = function()
{

	this.form[0].attr(this.boxattr1);
	this.form[2].css(this.txtcss);
	this.minichart[1].yAxis[0].update(
			{
				labels: {
		            style: {color : '#577295'}
		        }
			});
	this.minichart[2].yAxis[0].update(
			{
				labels: {
		            style: {color : '#577295'}
		        }
			});
	this.minichart[0].yAxis[0].update(
			gauge1_yAxes_opt
	);
	this.minichart[0].pane[0].update(
			gauge1_pane_opt 
	);
	
};


FE.prototype.redraw = function()
{
	var containerDiv = document.getElementById("container");
	var xoffset = containerDiv.offsetLeft;
	var yoffset = containerDiv.offsetTop;
	this.cont_g1.style.left = xoffset+this.FEx+(this.FEboxwidth-this.gaugew)/2+"px";
	this.cont_g1.style.top = yoffset+this.FEy+this.FEgauge_yoffset+"px";
	this.cont_g2.style.left = xoffset+this.FEx+(this.FEboxwidth-this.gaugew)/2+"px";
	this.cont_g2.style.top = yoffset+this.FEy+this.FEgauge_yoffset+this.gaugeh+"px";
	this.cont_g3.style.left = xoffset+this.FEx+(this.FEboxwidth-this.gaugew)/2+"px";
	this.cont_g3.style.top = yoffset+this.FEy+this.FEgauge_yoffset+(4*this.gaugeh/3)+"px";
	console.log("now resizing at w="+this.FEboxwidth+" h="+this.FEboxhigh);
		
	this.form[0].attr(
			{
				x: this.FEx,
				y: this.FEy,
				width :  this.FEboxwidth,
				height : this.FEboxhigh
			});
	this.form[1].attr({
		x: this.FEx+(this.FEboxwidth-this.FE_iconwidth)/2,
		y: this.FEy + this.FEiconoffset, 
		width: this.FE_iconwidth, 
		height: this.FE_iconh
	});
	this.form[2].attr({
		x: this.FEx+this.FEboxwidth/2,
		y: this.FEy+this.FEtxt_yoffset
	});
	var tmp_xoff = this.FEx+this.FEboxwidth/2-this.oldFEx-this.oldwidth/2;
	for(var i=0; i< this.no_conn; i++) {
		this.form[3+i].attr({
			translateX: tmp_xoff
		});
	};
	
	this.minichart[0].setSize(this.gaugew,this.gaugeh);
	this.minichart[1].setSize(this.gaugew,this.gaugeh/3);
	this.minichart[2].setSize(this.gaugew,this.gaugeh/3);
	this.minichart[0].redraw();
	this.minichart[1].redraw();
	this.minichart[2].redraw();
};
