
// Menu Buider of the DROP GUI

var menuContent = [
		["Index", "/Home/Index"],
		["GAL", "/Home/Tree"],
		["Graphs", "/Home/Graphs"],
		["IP FIB", "/Home/Fib"],
		["OpenFlow", "/Home/Of"],
		["Logs", "/Home/Logs"]
];

$(document).ready(function ()
{
    for (var i = 1; i <= menuContent.length; ++i)
    {
        var entry = menuContent[i - 1];
        var htmlEntry;

        var m = document.location.href.match(/[^\/]+$/);

        if (m == null || m[0] != entry[1])
        {
            htmlEntry = '<li class=mnav' + i + '><a href=' + entry[1] + '>' + entry[0] + '</a></li>';
        }
        else
        {
            htmlEntry = '<li class=selected><a href=' + entry[1] + '>' + entry[0] + '</a></li>';
        }

        $('#main-nav').append(htmlEntry);
    };
});