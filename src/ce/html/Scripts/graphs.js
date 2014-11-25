
function DrawTrafficChart(divId)
{
    return new Highcharts.Chart(
    {
        chart: {
	        renderTo: divId,
	        type: 'areaspline',
            zoomType: 'x',
	    },
	    title: {
	        text: 'Traffic shares'
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
	            text: 'Traffic  [bps]'
	        }
	    },
	    tooltip: {
	        shared: true,
	        valueSuffix: ' bps'
	    },
	    credits: {
	        enabled: false
	    },
	    plotOptions: {
	        areaspline: {
	            stacked: 'percent',
                fillOpacity: 0.5,
                marker: {
                    enabled: false
                }
	        }
	    }
    });
}

function DrawConsumptionChart(divId)
{
    return new Highcharts.Chart(
    {
        chart: {
	        renderTo: divId,
	        type: 'spline',
            zoomType: 'x',
	    },
	    title: {
	        text: 'Energy Consumption'
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
	            text: 'Power Consumption [W]'
	        }
	    },
	    tooltip: {
	        shared: true,
	        valueSuffix: ' W'
	    },
	    credits: {
	        enabled: false
	    },
	    plotOptions: {
	        spline: {
	            lineWidth: 4,
                marker: {
                    enabled: false
                }
	        }
	    }
    });
}

function DrawCoresChart(divId)
{
    return new Highcharts.Chart(
    {
        chart: {
	        renderTo: divId,
	        type: 'spline',
            zoomType: 'x',
	    },
	    title: {
	        text: 'Active cores'
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
	            text: '# of Cores'
	        },
	        min: 0,
            max: 10
	    },
	    tooltip: {
	        shared: true,
	        valueSuffix: ' cores'
	    },
	    credits: {
	        enabled: false
	    },
	    plotOptions: {
	        spline: {
	            lineWidth: 4,
                marker: {
                    enabled: false
                }
	        }
	    }
    });
}

function DrawXLPChart(divId)
{
    return new Highcharts.Chart(
    {
        chart: {
            renderTo: divId,
            alignTicks: false,
            zoomType: 'x',
	    },
	    title: {
	        text: 'Advanced Power Management'
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
        yAxis: [{
                title: {
                    text: 'Voltage [mV]',
                }
            },
            {
                title: {
                    text: '# of Cores',
                    },
                opposite: true,
                min: 0,
	    	    max: 8
            }],
	    tooltip: {
	        shared: true,
	    },
	    credits: {
	        enabled: false
	    }
    });
}

function UpdateChart(chart, name, data, redraw)
{
    var bshift = false;
    var series = chart.get(name);

    if (series == null)
    {
        return;
    }

	if (series.data.length > 100)
	{
        bshift = true;
	}

	series.addPoint(data, redraw, bshift, true);
}

function Update(tc, cc, lc, xc)
{
    $.getJSON("/json/graphs_update", function (data)
	{
        $.each(data.Elements, function (i, item)
        {
            if (item.Traffic != null)
            {
                UpdateChart(tc, item.Name, item.Traffic, true);
            }
            
            if (item.Consumption)
            {
                UpdateChart(cc, item.Name, item.Consumption, true);
            }

            if (item.Cores != null)
            {
                UpdateChart(lc, item.Name, item.Cores, true);
            }

            if (item.Load != null)
            {
                UpdateChart(xc, "Voltage", item.Load.Voltage, false);

                var freqs = item.Load.Freqs;

                if (freqs != null)
                {
                    $.each(freqs, function (i, freq)
                    {
                        UpdateChart(xc, freq.Name, freq.Data, false);
                    });
                }

                xc.redraw();
            }
        });
    });
}

$(function ()
{
	document.getElementById('container').style.width = "0%";
	document.getElementById('leftPanel').style.width = "50%";
	document.getElementById('rightPanel').style.width = "50%";
	
	var divG1 = document.createElement("div");

	divG1.id = "divG1";
	divG1.style.height = "48%";
	divG1.style.width = "95%";
	divG1.style.marginTop = "20px";
	divG1.style.marginLeft = "20px";
	divG1.style.marginBottom = "20px";
	divG1.style.marginRight = "20px";
	
	document.getElementById('leftPanel').appendChild(divG1);
	
	var divG2 = document.createElement("div");

	divG2.id = "divG2";
	divG2.style.height = "48%";
	divG2.style.width = "95%";
	divG2.style.marginTop = "20px";
	divG2.style.marginLeft = "20px";
	divG2.style.marginTop = "20px";
	divG2.style.marginLeft = "20px";
	divG2.style.marginBottom = "20px";
	divG2.style.marginRight = "100px";
	
	document.getElementById('rightPanel').appendChild(divG2);
	
	var divG3 = document.createElement("div");

	divG3.id = "divG3";
	divG3.style.height = "48%";
	divG3.style.width = "95%";
	divG3.style.marginTop = "20px";
	divG3.style.marginLeft = "20px";
	
	document.getElementById('leftPanel').appendChild(divG3);
	
	var divG4 = document.createElement("div");

	divG4.id = "divG4";
	divG4.style.height = "48%";
	divG4.style.width = "95%";
	divG4.style.marginTop = "20px";
	divG4.style.marginLeft = "20px";
	divG4.style.marginRight = "100px";
	
	document.getElementById('rightPanel').appendChild(divG4);
	
	var tc = DrawTrafficChart(divG1.id);
	var cc = DrawConsumptionChart(divG2.id);
	var lc = DrawCoresChart(divG3.id);
	var xc = DrawXLPChart(divG4.id);

    $.getJSON("/json/graphs_start", function (data)
	{
        $.each(data.Elements, function (i, item)
        {
            if (item.Traffic != null)
            {
                tc.addSeries({ id: item.Name, name: item.Name, data: item.Traffic }, false);
            }

            if (item.Consumption != null)
            {
                cc.addSeries({ id: item.Name, name: item.Name, data: item.Consumption }, false);
            }

            if (item.Cores != null)
            {
                lc.addSeries({ id: item.Name, name: item.Name, data: item.Cores }, false);
            }

            if (item.Load != null)
            {
                var freqs = item.Load.Freqs;

                if (freqs != null)
                {
                    $.each(freqs, function (i, freq)
                    {
                        xc.addSeries({
                            id: freq.Name,
                            name: freq.Name,
                            data: freq.Data,
                            type: 'area',
                            tooltip: { valueSuffix: ' cores' },
                            marker: { enabled: false },
                            stacking: 'normal',
                            fillOpacity: 0.5,
                            yAxis: 1
                        }, false);
                    });
                }

                xc.addSeries({
                    id: "Voltage",
                    name: "Voltage",
                    data: item.Load.Voltage,
                    type: 'spline',
                    color: '#000000',
                    tooltip: { valueSuffix: ' mV' },
                    marker: { enabled: false },
                    yAxis: 0
                }, false);
            }
        });

        tc.redraw();
        cc.redraw();
        lc.redraw();
        xc.redraw();

        setInterval(function ()
        {
            Update(tc, cc, lc, xc);
        }, 3000);
    });
});
