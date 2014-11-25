/**
 * 
 */

$(function () {
console.log("-----1");
jQuery("#list4").jqGrid({
	datatype: "local",
	height: 250,
   	colNames:['Entry No','Network', 'Mask', 'Gateway','Interface','Metric','Flags'],
   	colModel:[
   		{name:'id',index:'id', width:60, sorttype:"int"},
   		{name:'net',index:'net', width:100},
   		{name:'mask',index:'mask', width:50, sorttype:"int"},
   		{name:'gw',index:'gw', width:100, align:"right"},
   		{name:'iface',index:'iface', width:80},		
   		{name:'metric',index:'metric', width:50,align:"right"},		
   		{name:'flags',index:'flags', width:50, sortable:false}		
   	],
   	rowNum:10, 
   	rowList:[10,20,30],
   	multiselect: true,
   	caption: "Routing Table",
   	pager: '#pager5'
   		
}).navGrid("#pager5",{edit:true,add:true,del:true});
console.log("-----2");
var mydata = [
		{id:"1",net:"130.251.17.1",mask:"24",gw:"0.0.0.0",iface:"deth0",metric:"1",flags:"U"},
		{id:"2",net:"130.251.17.1",mask:"16",gw:"130.251.17.1",iface:"deth1",metric:"10",flags:"UG"}
		];
for(var i=0;i<=mydata.length;i++)
	jQuery("#list4").jqGrid('addRowData',i+1,mydata[i]);
console.log("-----3");

});