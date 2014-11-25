
function ShowFlows()
{
    $.getJSON("/json/openflow", function (data)
    {
        jQuery("#list4").jqGrid('clearGridData', false);

        $.each(data.Flows, function (i, item)
        {
            var f = "";

            $.each(item.Filters, function (i, filter)
            {
                for (var key in filter)
                {
                    f += key + ":" + filter[key] + ", ";
                }
            });

            f = f.substring(0, f.length - 2);

            var a = "";

            $.each(item.Actions, function (i, action)
            {
                for (var key in action)
                {
                    a += key + ":" + action[key] + ", ";
                }
            });

            a = a.substring(0, a.length - 2);

            var d = { filters: f, actions: a };
            jQuery("#list4").jqGrid('addRowData', i+1, d);
        });
    });
}

$(function ()
{
    jQuery("#list4").jqGrid(
    {
        datatype: "local",
        height: 500,
        width: 1200,
        colNames:['Filters', 'Actions'],
        colModel:[
   	        {name:'filters',index:'filters',width:50},
   	        {name:'actions',index:'actions'}		
        ],
        rowNum:10, 
        rowList:[10,20,30],
        multiselect: true,
        caption: "Openflow Table",
        pager: '#pager5'
   		
    }).navGrid("#pager5", { edit: true, add: true, del: true });

    ShowFlows();

    setInterval(function ()
    {
        ShowFlows();
    }, 5000);
});
