/**
 * The phy view main JS file of the DROP GUI
 */
var xc, yc;
//	var phyview_w = 0.7, phyview_h = 1;
var x_padding = 25;
var y_padding = 25;
var hbox_hperc = 1/6; 	//relative height of the horizontal box (IE1)
var hbox_wperc = 1;		//relative width of the horizontal box (IE1)
var vbox_hperc = 4/6;	//relative height of the vertical boxes
var vbox_wperc = 2/8;	//relative width of the vertical boxes
var FElist, IElist;

var FE1x = x_padding, FE2x = 200, FE3x = 375;
var chartTraffic, chartEAS;

function resizeIE(ie)
{
	if(ie.layout=='horizontal')
		{
			ie.IEx=x_padding;
			ie.IEy=y_padding;
			ie.IEboxwidth=xc - 2 * x_padding;
			ie.IEboxhigh=(yc - 2 * y_padding) * hbox_hperc;
			ie.IEicon_xoffset= (xc - 4 * x_padding) - ie.IE_iconwidth;
			ie.IEicon_yoffset= ie.IEy + ie.IEboxhigh/2 - ie.IE_iconh;
			ie.IEtxt_xoffset=2*x_padding;
			ie.IEtxt_yoffset=  ie.IEboxhigh/2 + y_padding;
			
		};
		
		if(ie.layout=='vertical')
		{
			ie.IEx=(xc * 3* vbox_wperc) + x_padding;
			ie.IEy=yc * (1- vbox_hperc);
			ie.IEboxwidth=xc*vbox_wperc - x_padding;
			ie.IEboxhigh=yc * vbox_hperc - y_padding;
			ie.IEicon_xoffset= (ie.IEboxwidth - ie.IE_iconwidth)/4;
			ie.IEicon_yoffset= ie.IEboxhigh - ie.IE_iconh - y_padding;
			
			ie.IEtxt_xoffset=ie.IEboxwidth/2;
			ie.IEtxt_yoffset= 2* y_padding;
			ie.IE1boxy = y_padding + (yc - 2 * y_padding) * hbox_hperc;
		};
};

function resizeFE(fe, index)
{
			if(index > 1) {
				fe.FEx=(xc * (index-1)* vbox_wperc) + x_padding;
			} else {
				fe.FEx=x_padding;
			}
			fe.FEy=yc * (1- vbox_hperc);
			fe.FEboxwidth=xc*vbox_wperc -  x_padding;
			fe.FEboxhigh=yc * vbox_hperc - y_padding;
			fe.FEicon_xoffset= (fe.FEboxwidth - fe.FE_iconwidth)/4;
			fe.FEicon_yoffset= fe.FEboxhigh - fe.FE_iconh - y_padding;
			
			fe.FEtxt_xoffset=fe.FEboxwidth/2;
			fe.FEtxt_yoffset= 2* y_padding;
			fe.gaugew = fe.FEboxwidth -10;
			fe.IE1boxy = y_padding + (yc - 2 * y_padding) * hbox_hperc;
};

function redraw(renderer) {
	yc = $('#container').height();
	xc = $('#container').width();
	  //resize just happened, pixels changed
	renderer.setSize(xc,yc);
	console.log("redrawing");
	for(var i=0; i<IElist.length; i++)
	{
	resizeIE(IElist[i]);
	IElist[i].redraw();
	};
	for(var i=0; i<FElist.length; i++)
		{
		resizeFE(FElist[i], i+1);
		FElist[i].redraw();
		}
};

function DrawEASChart(divObj)
{
//	console.log('******************** '+$('#'+divObj.id).height());
	chartEAS = new Highcharts.Chart(
		{
	        chart: {
	        	renderTo : divObj.id,
//	        	height: $('#'+divObj.id).height(),
//				width: $('#'+divObj.id).width()
	        	backgroundColor: null,
	        	plotBackgroundColor: 'white'
	        },
	        credits: {enabled: false},
	        title: {
                text: 'Committed Energy Profile',
                x: -20 //center
            },
            xAxis: {
            	title: {
                    text: 'Traffic Load [%]'
                },
                plotLines: [{
                    color: 'rgba(68, 170, 213, .2)',
                    width: 2,
                    value: 20,
                    id: "pl"
                }],
	            gridLineWidth: 2,
	            minorGridLineColor: '#E0E0E0',
	            minorGridLineWidth: 1,
	            minorTickLength: 0,
	            minorTickInterval: 'auto'
            },
            yAxis: {
                title: {
                    text: 'Power Consumption [W]'
                },
                gridLineWidth: 2,
                minorGridLineColor: '#E0E0E0',
                minorGridLineWidth: 1,
                minorTickLength: 0,
                minorTickInterval: 'auto',
                plotLines: [{
                    value: 0,
                    width: 2,
                    color: '#808080'
                }]
            },
            tooltip: {
                valueSuffix: 'W'
            },
            legend: {
            	enabled: false
            },
            series: [{
                name: 'Profile',
                data: [[0,7.0], [5,6.9], [10,9.5], [15,14.5], [20,18.2], [25,21.5], [30,25.2], [35,27.5], [40,28.3], 
                       [45,29.3], [50,30], [55,30.5], [60,30.8], [65,31], [70,31.1], [75,31.2], [80,31.2], [85,32],[90,32],[95,32],[100,32]]
            }]
		}
	);
	
	setInterval(function() { 

		chartEAS.xAxis[0].removePlotLine('pl');
		var traffic= Math.random()*100;
		chartEAS.xAxis[0].addPlotBand({
			color: 'rgba(68, 170, 213, .2)',
            width: 20,
            value: traffic,
            id: "pl"});

	}, 3000);
	
};

function DrawTrafficChart(divObj)
{
	chartTraffic = new Highcharts.Chart(
		{
	        chart: {
	        	renderTo : divObj.id,
	        	type:'areaspline',
	        	backgroundColor: null,
	        	plotBackgroundColor: 'white',
	        	animation: {
	                duration: 2500,
	                easing: 'linear'
	            },
	        	events: {
                    load: function() {
                    	//console.log("********** event load");
                        // set up the updating of the chart each second
                        var series = this.series[0];
                        setInterval(function() {
                        	var x = (new Date()).getTime(),
                                y = Math.random();
                        	var bshift = false;
                        	if(series.data.length > 100)
                        		bshift = true;
                            series.addPoint([x, y], true, bshift, true);
                         }, 3000);
                    }
                }
	        },
	        credits: {enabled: false},
	        title: {
                text: 'Incoming Traffic',
//                x: -10, //center
            },
             xAxis: {
            	type: 'datetime',
                tickPixelInterval: 50,
            	title: {
                    text: 'Time'
                },
            dateTimeLabelFormats:
            	{
            		second: '%M:%S'
            	},
        	gridLineWidth: 2,
            minorGridLineColor: '#E0E0E0',
            minorGridLineWidth: 1,
            minorTickLength: 0,
            minorTickInterval: 'auto',
            plotLines: [{
                value: 0,
                width: 2,
                color: '#808080'
            	}]
            },
            yAxis: {
                title: {
                    text: 'Load [Gbps]'
                },
                gridLineWidth: 2,
                minorGridLineColor: '#E0E0E0',
                minorGridLineWidth: 1,
                minorTickLength: 0,
                minorTickInterval: 'auto',
                plotLines: [{
                    value: 0,
                    width: 2,
                    color: '#808080'
                }]
            },
            tooltip: {
                valueSuffix: 'Gbps'
            },
            legend: {
            	enabled: false
            },
            
            series: [{
            	name: 'Traffic',
            	marker: {enabled: false},
            	
                data: (function() {
                    var data = [],
                        time = (new Date()).getTime(),
                        i;
                    for (i = 49; i >= 0; i = i -1) {
                        data.push({
                            x: time - i * 3000,
                        	//x: i,
                            y: Math.random()
                        });
                    };
                    return data;
                })()
            }]
		}
	);
};



$(function() {

	yc = $('#container').height();
	xc = $('#container').width();
	
	/*
	 * 		Tuning the right and left panels
	 */
	document.getElementById('leftPanel').style.width = "0%";
	document.getElementById('rightPanel').style.width = "30%";
	
	/*
	 * 		Drawing the right panel
	 */		
	var rightContent = document.createElement("div");
	rightContent.id = "rightContent";
	document.getElementById('rightPanel').appendChild(rightContent);
	var RContnet_Chart1Div = document.createElement("div");
	RContnet_Chart1Div.id = "RContnet_Chart1Div";
	RContnet_Chart1Div.style.height = "50%";
	RContnet_Chart1Div.style.width = "100%";
	//RContnet_Chart1Div.innerHTML = "Ciao1";
	rightContent.appendChild(RContnet_Chart1Div);

	DrawEASChart(RContnet_Chart1Div);
	
	var RContnet_Chart2Div = document.createElement("div");
	RContnet_Chart2Div.id = "RContnet_Chart2Div";
	RContnet_Chart2Div.style.height = "50%";
	RContnet_Chart2Div.style.width = "100%";
	//RContnet_Chart2Div.innerHTML = "Ciao2";
	rightContent.appendChild(RContnet_Chart2Div);
	
	DrawTrafficChart(RContnet_Chart2Div);

	/*
	 * 		Drawing the main view
	 */	
	var renderer = new Highcharts.Renderer($('#container')[0], xc , yc);

	var IE1 = new IE("IE1",renderer, "horizontal", true);
	IE1.IEtxt_xoffset=75;
	IE1.IEtxt_yoffset=70;
	resizeIE(IE1);
	IE1.draw();
	
	var IE2 = new IE("IE2",renderer, "vertical", false);
	IE2.IEx= 525;
	IE2.IEy = 200;
	IE2.no_conn = 4;
	IE2.IEboxwidth=225;
	IE2.IEboxhigh=300;
	IE2.IEtxt_xoffset=75;
	IE2.IEtxt_yoffset=60;
	IE2.IEicon_xoffset=0;
	IE2.IEicon_yoffset=200;
	IE2.IEicon= 'pronto.png';
	IE2.IE_iconwidth=200;
	resizeIE(IE2);
	IE2.draw();
	IElist = Array(IE1, IE2);
	
	var FE1 = new FE("FE1",renderer, FE1x);
//	FE1.lURL="http://www.google.it";
	resizeFE(FE1,1);
	FE1.draw();
	var FE2 = new FE("FE2",renderer, FE2x);
//	FE2.lURL="http://www.google.it";
	resizeFE(FE2,2);
/*	var FE3 = new FE("FE3",renderer, FE3x);
	FE3.no_conn = 2;
	FE3.lURL="http://www.google.it";
	resizeFE(FE3,3);
	FElist = Array(FE1, FE2, FE3);
*/
	/*
	 * 		Adding some animations
	 */	
	window.setTimeout(function() {FE1.standby();}, 3000);
	window.setTimeout(function() {FE1.wakeup();}, 6000);
	window.setTimeout(function() {FE2.draw();}, 7000);
//	window.setTimeout(function() {FE3.draw();}, 8000);
	window.setTimeout(function() {
		var FE3 = new FE("FE3",renderer, FE3x);
		FE3.no_conn = 2;
//		FE3.lURL="http://www.google.it";
		resizeFE(FE3,3);
		FElist = Array(FE1, FE2, FE3);
		FE3.draw();
	}, 8000);
//		window.setTimeout(function() {FE1.disappear();}, 9000);
//		window.setTimeout(function() {FE1.appear();}, 12000);
	//		setTimeout(FE1.standby(), 5000);
//		setTimeout(FE1.wakeup(), 10000);
	
	/*
	 * 		Handling page resizing
	 */	
	$(window).resize(function() {
		redraw(renderer);
		});
	$(document).on('webkitfullscreenchange mozfullscreenchange fullscreenchange',function() { 
		redraw(renderer);
		});
});
		
		
		

	
	
