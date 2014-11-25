/**
 * 
 */

function DrawG1(divId)
{
	chart = new Highcharts.Chart({
	       // $('#'+divG1.id).highcharts({
	            chart: {
	            	renderTo: divId,
	                type: 'areaspline'
	            },
	            title: {
	                text: 'Energy Consumption'
	            },
	            legend: {
	                layout: 'vertical',
	                align: 'right',
	                verticalAlign: 'middle',
	                borderWidth: 0,
	                backgroundColor: '#FFFFFF'
	            },
	            xAxis: {
	                categories: [
	                    'Monday',
	                    'Tuesday',
	                    'Wednesday',
	                    'Thursday',
	                    'Friday',
	                    'Saturday',
	                    'Sunday'
	                ],
	                plotBands: [{ // visualize the weekend
	                    from: 4.5,
	                    to: 6.5,
	                    color: 'rgba(68, 170, 213, .2)'
	                }]
	            },
	            yAxis: {
	                title: {
	                    text: 'Power Consumption [W]'
	                }
	            },
	            tooltip: {
	                shared: true,
	                valueSuffix: ' units'
	            },
	            credits: {
	                enabled: false
	            },
	            plotOptions: {
	                areaspline: {
	                    fillOpacity: 0.5
	                }
	            },
	            series: [{
	                name: 'John',
	                data: [3, 4, 3, 5, 4, 10, 12]
	            }, {
	                name: 'Jane',
	                data: [1, 3, 4, 3, 3, 5, 4]
	            }]
	        });
};



$(function () {
	document.getElementById('container').style.width = "0%";
	document.getElementById('leftPanel').style.width = "49%";
	document.getElementById('rightPanel').style.width = "49%";
	
	
	var divG1 = document.createElement("div");
	divG1.id = "divG1";
	divG1.style.height = "50%";
	divG1.style.width = "99%";
	divG1.style.marginTop = "20px";
	divG1.style.marginLeft = "20px";
	divG1.style.marginRight = "20px";
	
	document.getElementById('leftPanel').appendChild(divG1);
	
	var divG2 = document.createElement("div");
	divG2.id = "divG2";
	divG2.style.height = "50%";
	divG2.style.width = "99%";
	divG2.style.marginTop = "20px";
	divG2.style.marginLeft = "20px";
	divG2.style.marginTop = "20px";
	divG2.style.marginLeft = "20px";
	
	divG2.style.marginRight = "100px";
	
	document.getElementById('rightPanel').appendChild(divG2);
	
	var divG3 = document.createElement("div");
	divG3.id = "divG3";
	divG3.style.height = "49%";
	divG3.style.width = "99%";
	divG3.style.marginTop = "20px";
	divG3.style.marginLeft = "20px";
	divG3.style.marginBottom = "20px";
	document.getElementById('leftPanel').appendChild(divG3);
	
	var divG4 = document.createElement("div");
	divG4.id = "divG4";
	divG4.style.height = "49%";
	divG4.style.width = "99%";
	divG4.style.marginTop = "20px";
	divG4.style.marginLeft = "20px";
	divG4.style.marginBottom = "20px";
	document.getElementById('rightPanel').appendChild(divG4);
	
	DrawG1(divG1.id);
	DrawG1(divG2.id);
	DrawG1(divG3.id);
	DrawG1(divG4.id);

    });
    
