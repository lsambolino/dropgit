
function ShowRoutes()
{
    $.getJSON("/json/ip_fib", function (data)
    {
        jQuery("#list4").jqGrid('clearGridData', false);

        $.each(data.Routes, function (i, item)
        {
            var d = { net: item.Network, mask: item.Prefix, gw: item.Gateway, iface: item.Interface, metric: item.Metric, flags: item.Flags };
            jQuery("#list4").jqGrid('addRowData', i+1, d);
        });
    });
}

$(function ()
{
    jQuery("#list4").jqGrid(
    {
        datatype: "local",
        height: 300,
        width: 800,
        colNames:['Network', 'Mask', 'Gateway','Interface','Metric','Flags'],
        colModel:[
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
   		
    }).navGrid("#pager5", { edit: true, add: true, del: true });

    ShowRoutes();

    setInterval(function ()
    {
        ShowRoutes();
    }, 5000);
});
