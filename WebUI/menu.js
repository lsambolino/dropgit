/**
 * Menu Buider of the DROP GUI
 */

var menuContent = [
		["Home", "index.html"],
		["GAL", "tree.html"],
		["Graphs", "graphs.html"],
		["IP FIB", "ipfib.html"],
		["OpenFlow", "of.html"],
		["Logs", "logs.html"]
];

$(document).ready(function() {
	
	 for(var i=1; i<=menuContent.length; i++)
		{
			var entry = menuContent[i-1], htmlEntry;
			if ( document.location.href.match(/[^\/]+$/)[0] != entry[1])
				{
					htmlEntry = '<li class=mnav'+i+'><a href='+entry[1]+'>'+entry[0]+'</a></li>';
				} else {
					htmlEntry = '<li class=selected><a href='+entry[1]+'>'+entry[0]+'</a></li>';
				}
			
			$('#main-nav').append(htmlEntry);
		};
	
});