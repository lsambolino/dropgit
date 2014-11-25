/**
 *  Libraries for the DROP GUI 
 */

/*** Let's define some useful functions ***/
Array.prototype.addToRender=function()
	{
  		for (var i=0;i<this.length;i++)
  		{this[i].add();}
	};
Array.prototype.animateRender=function(attr,time)
{
	for (var i=0;i<this.length;i++)
	{this[i].animate(attr,time);}
};
Array.prototype.AddLink=function(link)
{
	for (var i=0;i<this.length;i++)
	{this[i].on('click', function() {
		location.href = link;
    });}
};

/*** Var Init and Default Settings ***/
var animate_Appear_attr = { opacity: 1 };
var animate_Disppear_attr = { opacity: 0 };
var animatetime = { duration: 1000 };
var labelColor = 'rgb(255,255,255)';
var boxattr1 = {
		fill : {
			radialGradient: { cx: 0, cy: 0.3, r: 1 },
	 		stops: [ 
	 	         [0, 'blue'], 
	 	         [1, 'gray'] ]
		},
		zIndex : 3,
		opacity: 0
	};
var imgattr = {
		zIndex : 3,
		opacity: 0
	};
var txtattr = {
        align: 'center',
        zIndex: 5,
        padding: 0,
        opacity: 0
    };
var txtcss = {
        fontSize: '50px',
        color: labelColor
    };

var gauge1_yAxes_opt = {
    min: 0, max: 100,
    minorTickInterval: 'auto',
    minorTickWidth: 0,
    minorTickLength: 10,
    minorTickPosition: 'inside',
    minorTickColor: '#666',

    tickPixelInterval: 30,
    tickWidth: 1,
    tickPosition: 'inside',
    tickLength: 15,
    tickColor: '#666',
    labels: {
        step: 2,
        rotation: 'auto',
        style: {color : '#577295'}
    },
    title: {text: null}, //'Power [%]'},
    plotBands: [
                {
                	from: 0, to: 100, 
                	color:{
                		linearGradient: { x1: 0, y1: 0, x2: 1, y2: 0},
                		stops: [[0, 'green'],[.4, 'yellow'],[.7, 'yellow'],[.8, 'orange'],[1,'red'] ]
                		}
                }
                
                /*{
    	from: 0, to: 50, color: '#55BF3B' // green
    }, {
        from: 50, to: 80, color: '#DDDF0D' // yellow
    }, {
        from: 80, to: 100, color: '#DF5353' // red
    }*/]        
};

var gauge1_pane_opt = {
    startAngle: -130,
    endAngle: 130,
    background: [{
        backgroundColor: {
            linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
            stops: [[0, '#FFF'], [1, '#333']]
        },
        borderWidth: 0,
        outerRadius: '109%'
    }, {
        backgroundColor: {
            linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
            stops: [[0, '#333'],[1, '#FFF']]
        },
        borderWidth: 5,
        outerRadius: '107%'
    }, {
        // default background
    }, {
        backgroundColor: '#DDD',
        borderWidth: 0,
        outerRadius: '105%',
        innerRadius: '103%'
    }]
};

function get_gauge_option(containerId) {
	var opt={
        chart: {
        	renderTo : containerId,
            type: 'gauge',
            backgroundColor: null,
            plotBackgroundColor: null,
	        plotBackgroundImage: null,
	        plotBorderWidth: 0,
	        plotShadow: false,
        },
        title: {text: false},
        credits: {enabled: false},
        exporting: {enabled: false},
        pane: gauge1_pane_opt,
	    // the value axis
	    yAxis: gauge1_yAxes_opt,
	    plotOptions: {
            series: {
            	dataLabels: { enabled: false }
            },
	        gauge: {
				dial: {
					radius: '90%',
					backgroundColor: {
			            linearGradient: {
			                x1: 0,
			                y1: 0,
			                x2: 1,
			                y2: 0
			            },
			            stops: [ //[ 0.35, '#7070B8' ], [0, '#D69999'],
			                                           [0.3, '#B84D4D'],
			                                           [0.45, '#7A0000'],
			                                           [0.55, '#7A0000'],
			                                           [0.7, '#B84D4D'],
			                                           [1, '#D69999']]
			        },
					borderColor: '#7070B8',
					borderWidth: 0,
					baseWidth: 3,
					topWidth: 1,
					baseLength: '70%', // of radius
					rearLength: '25%'
				}
			}
	    },
	    series: [{
	    	name: 'Load', data: [0] }]
	};
	return opt;
}

function get_hgauge_option(containerId, nameSerie) {
	var opt={
    chart: {
        renderTo: containerId,
        defaultSeriesType: 'bar',
        backgroundColor: null,
        
        plotBorderWidth: 2,
        plotBackgroundColor: '#FFF',
        plotBorderColor: '#D8D8D8',
        plotShadow: false,
 //       spacingBottom: 43,
 //       width: 350
    },
    credits: { enabled: false},
    exporting: {enabled: false},
    xAxis: {
        labels: { enabled: false },
         tickLength: 0
    },
    title: {text: null},
    legend: {enabled: false},
    yAxis: {
        title: {text: null},
        labels: {y: 20},
        min: 0,
        max: 100,
        tickInterval: 20,
        minorTickColor: '#FFF',
        TickColor: '#FFF',
        minorTickInterval: 10,
        tickWidth: 1,
        tickLength: 8,
        minorTickLength: 5,
        minorTickWidth: 1,
        minorGridLineWidth: 0,
        labels: {style: {color : '#577295'}}
    },
    tooltip: {
        formatter: function() {
            return nameSerie+' '+ this.y +'%';
        }
    },
    plotOptions: {},
    series: [{
        borderColor: '#7070B8',
        borderRadius: 3,
        borderWidth: 1,
        color: {
            linearGradient: {
                x1: 0,
                y1: 0,
                x2: 1,
                y2: 0
            },
            stops: [ //[ 0.35, '#7070B8' ], [0, '#D69999'],
                                           [0.3, '#B84D4D'],
                                           [0.45, '#7A0000'],
                                           [0.55, '#7A0000'],
                                           [0.7, '#B84D4D'],
                                           [1, '#D69999']]
        },
        pointWidth: 50,
        data: [0]}]      
	};
	return opt;
}

/*
function FEdisappear() {
	this.form.animateRender(animate_Disppear_attr,1);
};

function FEAppear() {
	this.form.animateRender(animate_Appear_attr,1);
};
*/
/*var container_chartCorrectiveAction = new Highcharts.Chart({
chart: {
        renderTo: 'containerg',

                type: 'bar',
                height: 150,
                width: 200,
                backgroundColor: null

            },
            title: null,
            xAxis: {
                categories: ['Rx', 'Tx'],
                title: {
                    text: 'Traffic'
                },
                labels: {
                    style: {
                        width: '100px'
                    }
                }
            },
            yAxis: {
                min: 0,
                title: {
                    text: '[%]',
                    align: 'high'
                },
                labels: {
                    overflow: 'justify'
                }
            },
            tooltip: {
                formatter: function() {
                    return ''+ this.series.name +': '+ this.y +' [%]';
                }
            },
            plotOptions: {
                bar: {
                    dataLabels: {
                        enabled: true
                    }
                },
                series: {
                    pointWidth:10,
                    groupPadding: .05,
                    shadow: true
                }
            },
            legend: {
                layout: 'horizontal',
                align: 'center',
                verticalAlign: 'bottom',
                floating: false,
                borderWidth: 1,
                backgroundColor: '#FFFFFF',
                shadow: true,
                labelFormatter: function() {
                    return '<div class="' + this.name + '-arrow"></div><span style="font-family: \'Advent Pro\', sans-serif; font-size:12px">' + this.name +'</span><br/><span style="font-size:10px; color:#ababaa">   Total: ' + this.options.total + '</span>';
                }
            },
            credits: {
                enabled: false
            },
            exporting: { 
                enabled: false 
            },
            series: [{
        name: 'Heavy',
        total: '0',
        data: [null]
        },{
        name: 'Intermediate',
        total: '0',
        data: [null]
        },{
        name: 'Line',
        total: '0',
        data: [null]
        },{
        name: 'Lite',
        total: '1',
        data: [1]
        }]
        });
*/
