
// The phy view main JS file of the DROP GUI

var xc;
var yc;

//	var phyview_w = 0.7, phyview_h = 1;
var x_padding = 25;
var y_padding = 25;

var hbox_hperc = 1 / 6; 	//relative height of the horizontal box (IE1)
var hbox_wperc = 1;		//relative width of the horizontal box (IE1)
var vbox_hperc = 4 / 6;	//relative height of the vertical boxes
var vbox_wperc = 2 / 8;	//relative width of the vertical boxes

var FElist = new Array();
var IElist = new Array();

var ie_idx = 0;
var fe_idx = 0;

var x_FE_len = 175;

var chartTraffic;
var chartEAS;

function resizeIE(ie)
{
    if (ie.layout == 'horizontal')
    {
        ie.IEx = x_padding;
        ie.IEy = y_padding;
        ie.IEboxwidth = xc - 2 * x_padding;
        ie.IEboxhigh = (yc - 2 * y_padding) * hbox_hperc;
        ie.IEicon_xoffset = (xc - 4 * x_padding) - ie.IE_iconwidth;
        ie.IEicon_yoffset = ie.IEy + ie.IEboxhigh / 2 - ie.IE_iconh;
        ie.IEtxt_xoffset = 2 * x_padding;
        ie.IEtxt_yoffset = ie.IEboxhigh / 2 + y_padding;
    }
    else if (ie.layout == 'vertical')
    {
        ie.IEx = (xc * 3 * vbox_wperc) + x_padding;
        ie.IEy = yc * (1 - vbox_hperc);
        ie.IEboxwidth = xc * vbox_wperc - x_padding;
        ie.IEboxhigh = yc * vbox_hperc - y_padding;
        ie.IEicon_xoffset = (ie.IEboxwidth - ie.IE_iconwidth) / 4;
        ie.IEicon_yoffset = ie.IEboxhigh - ie.IE_iconh - y_padding;

        ie.IEtxt_xoffset = ie.IEboxwidth / 2;
        ie.IEtxt_yoffset = 2 * y_padding;
        ie.IE1boxy = y_padding + (yc - 2 * y_padding) * hbox_hperc;
    }
};

function resizeFE(fe, index)
{
    if (index > 1)
    {
        fe.FEx = (xc * (index - 1) * vbox_wperc) + x_padding;
    }
    else
    {
        fe.FEx = x_padding;
    }

    fe.FEy = yc * (1 - vbox_hperc);
    fe.FEboxwidth = xc * vbox_wperc - x_padding;
    fe.FEboxhigh = yc * vbox_hperc - y_padding;
    fe.FEicon_xoffset = (fe.FEboxwidth - fe.FE_iconwidth) / 4;
    fe.FEicon_yoffset = fe.FEboxhigh - fe.FE_iconh - y_padding;

    fe.FEtxt_xoffset = fe.FEboxwidth / 2;
    fe.FEtxt_yoffset = 2 * y_padding;
    fe.gaugew = fe.FEboxwidth - 10;
    fe.IE1boxy = y_padding + (yc - 2 * y_padding) * hbox_hperc;
};

function redraw(renderer)
{
    yc = $('#container').height();
    xc = $('#container').width();

    //resize just happened, pixels changed
    renderer.setSize(xc, yc);
    console.log("redrawing");

    for (var i = 0; i < IElist.length; i++)
    {
        resizeIE(IElist[i]);
        IElist[i].redraw();
    };

    for (var i = 0; i < FElist.length; i++)
    {
        resizeFE(FElist[i], i + 1);
        FElist[i].redraw();
    }
};

function DrawEASChart(divObj)
{
    return new Highcharts.Chart(
	{
		chart: {
		    renderTo: divObj.id,
		    backgroundColor: null,
		    plotBackgroundColor: 'white'
		},
		credits: { enabled: false },
		title: {
		    text: 'Committed Energy Profile',
		    x: -20 //center
		},
		xAxis: {
		    title: {
		        text: 'Traffic Load [%]'
		    },
		    plotLines: [{
		        //color: 'rgba(68, 170, 213, .7)',
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
		}
	});
};

function DrawTrafficChart(divObj)
{
    return new Highcharts.Chart(
	{
		chart: {
		    renderTo: divObj.id,
		    backgroundColor: null,
		    plotBackgroundColor: 'white',
		    animation: {
		        duration: 2500,
		        easing: 'linear'
		    }
		},
		credits: { enabled: false },
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
		        text: 'Load [bps]'
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
		    valueSuffix: 'bps'
		},
		legend: {
		    enabled: false
		}
	});
};

function Update(tc, eas, max)
{
    $.getJSON("/json/index_update", function (data)
    {
        $.each(data.Elements, function (i, item)
        {
            FElist.forEach(function (entry)
            {
                if (entry.name == item.Name)
                {
                    if (item.Connected)
                    {
                        entry.wakeup();
                    }
                    else
                    {
                        entry.standby();
                    }

                    if (item.Load !== undefined)
                    {
                        entry.requestDataGauge(item.Load, 0);
                    }

                    if (item.Traffic !== undefined)
                    {
                        entry.requestDataGauge(item.Traffic, 1);
                    }

                    if (item.Consumption !== undefined)
                    {
                        entry.requestDataGauge(item.Consumption, 2);
                    }
                }
            });
        });

        if (data.Traffic !== undefined)
        {
            var series = tc.get("Traffic");

            if (series != null)
            {
                var len = series.data.length;

                if (series.data[len - 1][0] != data.Traffic[0])
                {
                    series.addPoint(data.Traffic, true, len > 100, true);

                    eas.xAxis[0].removePlotLine('pl');
                    eas.xAxis[0].addPlotBand({
                        color: 'rgba(68, 170, 213, .2)',
                        width: 20,
                        value: data.Traffic[1] / max * 100,
                        id: "pl"
                    });
                }
            }
        }
    });
}

$(function ()
{
    yc = $('#container').height();
    xc = $('#container').width();

    // Tuning the right and left panels
    document.getElementById('leftPanel').style.width = "0%";
    document.getElementById('rightPanel').style.width = "30%";

    // Drawing the right panel
    var rightContent = document.createElement("div");
    rightContent.id = "rightContent";
    document.getElementById('rightPanel').appendChild(rightContent);

    var RContnet_Chart1Div = document.createElement("div");
    RContnet_Chart1Div.id = "RContnet_Chart1Div";
    RContnet_Chart1Div.style.height = "50%";
    RContnet_Chart1Div.style.width = "100%";
    rightContent.appendChild(RContnet_Chart1Div);

    var RContnet_Chart2Div = document.createElement("div");
    RContnet_Chart2Div.id = "RContnet_Chart2Div";
    RContnet_Chart2Div.style.height = "50%";
    RContnet_Chart2Div.style.width = "100%";
    //RContnet_Chart2Div.innerHTML = "Ciao2";
    rightContent.appendChild(RContnet_Chart2Div);

    var eas = DrawEASChart(RContnet_Chart1Div);
    var tc = DrawTrafficChart(RContnet_Chart2Div);

    // Drawing the main view
    var renderer = new Highcharts.Renderer($('#container')[0], xc, yc);
    var max = 0;

    $.getJSON("/json/index_start", function (data)
    {
        $.each(data.Elements, function (i, item)
        {
            if (item.Type == "IE")
            {
                if (item.Layout == "horizontal")
                {
                    var ie = new IE(item.Name, renderer, item.Layout, true);

                    ie.IEtxt_xoffset = 75;
                    ie.IEtxt_yoffset = 70;
                    resizeIE(ie);
                    ie.draw();

                    IElist[ie_idx++] = ie;
                }
                else if (item.Layout == "vertical")
                {
                    var ie = new IE(item.Name, renderer, item.Layout, false);
                    ie.IEx = 525;
                    ie.IEy = 200;
                    ie.no_conn = 4;
                    ie.IEboxwidth = 225;
                    ie.IEboxhigh = 300;
                    ie.IEtxt_xoffset = 75;
                    ie.IEtxt_yoffset = 60;
                    ie.IEicon_xoffset = 0;
                    ie.IEicon_yoffset = 200;
                    ie.IEicon = '/Images/pronto.png';
                    ie.IE_iconwidth = 200;
                    resizeIE(ie);
                    ie.draw();

                    IElist[ie_idx++] = ie;
                }
            }
            else if (item.Type == "FE")
            {
                var fe = new FE(item.Name, renderer);
                resizeFE(fe, fe_idx + 1);
                fe.draw();

                FElist[fe_idx++] = fe;

                if (item.Connected)
                {
                    fe.wakeup();
                }
                else
                {
                    fe.standby();
                }

                if (item.Load !== undefined)
                {
                    fe.requestDataGauge(item.Load, 0);
                }

                if (item.Traffic !== undefined)
                {
                    fe.requestDataGauge(item.Traffic, 1);
                }

                if (item.Consumption !== undefined)
                {
                    fe.requestDataGauge(item.Consumption, 2);
                }
            }
        });

        if (data.Profile != null)
        {
            eas.addSeries({
                id: 'Profile',
                name: 'Profile',
                data: data.Profile,
                type: 'spline',
                tooltip: { valueSuffix: ' W' },
                marker: { enabled: true }
            }, false);

            eas.redraw();
        }

        if (data.Traffic != null)
        {
            max = data.Traffic.Max;

            tc.addSeries({
                id: 'Traffic',
                name: 'Traffic',
                data: data.Traffic.Data,
                type: 'areaspline',
                tooltip: { valueSuffix: ' bps' },
                marker: { enabled: false },
            }, false);

            tc.redraw();
        }

        window.setInterval(function() { Update(tc, eas, max); }, 3000);
    });

    $(window).resize(function ()
    {
        redraw(renderer);
    });

    $(document).on('webkitfullscreenchange mozfullscreenchange fullscreenchange', function ()
    {
        redraw(renderer);
    });
});
