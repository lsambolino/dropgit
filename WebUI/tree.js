/**
 * 
 */

// Global Variables
var tree, xc, yc, conny, selectedNode;
var tree_x = 1.;
var tree_y = 1.;
var boxw = 85;
var boxh = 40;

/* layers:
 * 0 - Logical Res 
 * 1 - The Root Node
 * 2 - First Level of Phy Res
 * 3 - Second Level of Phy Res
 * ....
 */

function SortByID(a, b)
{
    if(a.id < b.id) return -1;
    if(a.id > b.id) return 1;
    return 0;
};

function TreeNode(id, name) {
	this.name = name;
	this.img = '';
	this.id = id;
	this.completeId = id.split('.');
	this.x = 0;
	this.y = 0;
	this.connectors= new Array();
	this.img= "images/router.svg";
	this.layer = this.completeId.length;
	if (this.layer == 2) // we have to understand if it is a logical or a physical resource
		{
			if (this.completeId[0] == '1') // logical
				{
				this.layer = 0;
				
				}
			else {
				if (this.completeId[0] == '0') // phy
				{
				this.layer = 2;
				this.parentId = 0;
				}
			}
		} else {
			var parentId = '0';
			for (var i=1; i<this.layer -1; i++)
				{
					parentId += "." + this.completeId[i];
				};
				this.parentId = parentId;
		};
//		console.log("node " + this.name + " created, id="+this.id+" layer= " + this.layer + " --- id[0]= " + this.completeId[0]);
};


function Tree()
{
	this.NodeList = new Array();
	this.NodeLayers = new Array();
	//let's define the two mandatory levels
	this.NodeLayers[0] = new Array();
	this.NodeLayers[1] = new Array();
	//let's add the root node
	var rootNode = new TreeNode('0', 'DROP');
	this.NodeList.push(rootNode);
	this.NodeLayers[1].push(rootNode);
	
}

function TreefindParent(t_, n_)
{
	var node = 'No';
	var found = false;
//	console.log("----> findParent for node "+n_.name+" layer= "+n_.layer);
	if(n_.layer==0) {
		node = t_.NodeLayers[1][0];
	} else {
		if(n_.layer > 1) {
		for(var i=0; i<t_.NodeLayers[n_.layer -1].length; i++)
		{
//			console.log("----> findParent for node "+n_.name+" parentId="+n_.parentId+" now examining "+t_.NodeLayers[n_.layer -1][i].id);
			if(n_.parentId == t_.NodeLayers[n_.layer -1][i].id)
				{
				node = t_.NodeLayers[n_.layer -1][i];
				found == true;
				};
		};
	}};
	return node;
};

function TreefindById(t_, id_)
{
	var node = t_.NodeLayers[1][0];
	var splittedId = id_.split('.');
	var layer = splittedId.length;

//	console.log("id_: "+id_+" layer: "+layer+" splittedId[0]:"+splittedId[0]/*+" node.id: "+node.id*/);		
	if(layer == 2) {
		if(splittedId[0] == 1)
			layer = 0;
	}
	for(var i = 0; i < t_.NodeLayers[layer].length; i++)
	{
//		console.log("now examining "+t_.NodeLayers[layer][i].id);
		if(t_.NodeLayers[layer][i].id == id_) {
			node = t_.NodeLayers[layer][i];
			break;
		}
	}
//	console.log("id_: "+id_+" layer: "+layer+" node.id: "+node.id);		
	return node;
};

function TreefindChild(t_, n_)
{
	

	var nodes = new Array();
	var limit = t_.NodeLayers.length - 1;
//	console.log("----> findChild for node "+n_.name+" layer= "+n_.layer+" limit"+limit);
	if(n_.layer > 1 && n_.layer < limit) 
		for(var i=0; i<t_.NodeLayers[n_.layer +1].length; i++)
		{
//			console.log("----> findChild for node "+n_.name+" Id="+n_.id+" now examining "+t_.NodeLayers[n_.layer +1][i].id);
			if(n_.id == t_.NodeLayers[n_.layer +1][i].parentId)
				{
//				console.log("----> findChild for node "+n_.name+" Id="+n_.id+" now examining "+t_.NodeLayers[n_.layer +1][i].id);
				nodes.push(t_.NodeLayers[n_.layer +1][i]);
				};
		};
	if(n_.layer == 1)
	{
		for(var i=0; i<t_.NodeLayers[0].length; i++)
			nodes.push(t_.NodeLayers[0][i]);
		for(var i=0; i<t_.NodeLayers[2].length; i++)
			nodes.push(t_.NodeLayers[2][i]);
	};
	
	return nodes;
};

Tree.prototype.addNode = function(node) {
	var l = node.layer;
	if(l > this.NodeLayers.length )
		{
		console.log(node.name + " non inserted");
			return -1;
		};
//		console.log("NodeLayers.length= "+this.NodeLayers.length+"   node.layer= "+node.layer);
	if(l == this.NodeLayers.length )
		{
			this.NodeLayers[l] = new Array();
		};
		
		this.NodeList.push(node);
		this.NodeLayers[l].push(node);
};

function NodeCoo(tree)
{
	var treew = xc * tree_x - boxw;
	var treeh = yc * tree_y;
	var layer_distance= (treeh - (tree.NodeLayers.length * boxh)) / (tree.NodeLayers.length);
	
	var sortedarray;
	var nodeInLayer;
	for(var l=0; l<tree.NodeLayers.length; l++) {
		sortedarray = tree.NodeLayers[l].sort(SortByID);
		nodeInLayer = sortedarray.length;
		// check if there is room for placing all the nodes of this layer on the same row
		var room = Math.floor((nodeInLayer * boxw)/treew) + 1;
		
		var yOff_inLayer=0;
		for(var n=0; n<nodeInLayer; n++) {
			sortedarray[n].y = (l + .5) * (layer_distance + boxh)+(yOff_inLayer-room/2)*(boxh+5);
			sortedarray[n].x = treew / (nodeInLayer) /2 + n * (treew / (nodeInLayer));
			sortedarray[n].conny = layer_distance/3;
			if(room > 1)
				yOff_inLayer = (yOff_inLayer + 1) % room;
//			console.log("y="+sortedarray[n].y+" room="+room+" nodeInLayer="+nodeInLayer+" boxw="+boxw+" tree_x="+tree_x);	
//			console.log("::: " + sortedarray[n].name + "  x= " + sortedarray[n].x + "  y= " + sortedarray[n].y);
			//+ "||| layer_distance= "+layer_distance+" boxh= "+boxh+" tree.NodeLayers.length= "+tree.NodeLayers.length+" treeh= "+treeh);
		};
	};
};

function redrawTree(renderer)
{
	yc = $('#container').height();
	xc = $('#container').width();
	renderer.setSize(xc,yc);
	NodeCoo(tree);
/*	for(var m in tree.NodeList[2].connectors[0].element.pathSegList[0])
		console.log('### '+m);
	console.log('---1 '+tree.NodeList[2].connectors[0].element.pathLength);
	console.log('---2 '+tree.NodeList[2].connectors[0].element.pathSegList[0]);
	console.log('---2 '+tree.NodeList[2].connectors[0].element.pathSegList[1]);*/
	for(var l=0; l<tree.NodeList.length; l++) {
		tree.NodeList[l].rect.attr(
			{
				x: tree.NodeList[l].x,
				y: tree.NodeList[l].y
			}		
		);
		tree.NodeList[l].img_render.attr(
			{
				x: tree.NodeList[l].x + 5,
				y: tree.NodeList[l].y +5
			}
		);
		tree.NodeList[l].txt.attr(
			{
				x: tree.NodeList[l].x + 3*boxh/2,
				y: tree.NodeList[l].y + boxh/2+3
			}
		);
		
		var child = tree.NodeList[l];
		for(var j=0; j < tree.NodeList[l].connectors.length; j++)
		{
/*			for(var k in tree.NodeList[l].connectors[j].element.pathSegList.getItem(0))
				console.log('### '+k);
			console.log('#-- '+tree.NodeList[l].connectors[j].element.pathSegList.getItem(0).x);
*/
			var father = TreefindParent(tree, child);
//			console.log(l+" "+j+" "/*+tree.NodeList[l].connectors[j].element.pathSegList[0].x*/+" "+child.x);
			tree.NodeList[l].connectors[j].element.pathSegList.getItem(0).x = child.x + boxw/2;
			tree.NodeList[l].connectors[j].element.pathSegList.getItem(1).x = child.x + boxw/2;
			tree.NodeList[l].connectors[j].element.pathSegList.getItem(2).x = father.x + boxw/2;
			tree.NodeList[l].connectors[j].element.pathSegList.getItem(3).x = father.x + boxw/2;
			
			if(father.y < child.y)
			{
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(0).y = child.y;
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(1).y = father.y + father.conny + boxh;
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(2).y = father.y + father.conny + boxh;
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(3).y = father.y + boxh;
			} else {
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(0).y = child.y + boxh;
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(1).y = (child.y + boxh + father.y)/2;
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(2).y = (child.y + boxh + father.y)/2;
				tree.NodeList[l].connectors[j].element.pathSegList.getItem(3).y = father.y;
			};
		};
	};
};

function NodeHighlight(node)
{
	node.rect.attr({fill: 'red'});
	
	if(node.layer != 1)
		TreefindParent(tree, node).rect.attr({fill: 'orange'});
		
	for(var i=0; i< node.connectors.length; i++)
		node.connectors[i]
			.toFront()
			.attr({stroke: 'orange'});
	
	var childs = TreefindChild(tree, node);
	for(var i=0; i< childs.length; i++)
	{
		childs[i].rect.attr({fill: 'orange'});
		for(var j=0; j< childs[i].connectors.length; j++)
		{
			childs[i].connectors[j]
				.toFront()
				.attr({stroke: 'orange'});
		};
	};
};

function NodeUnlight(node)
{
	if(node.layer != 1)
		TreefindParent(tree, node).rect.attr({fill: 'white'});
	
	node.rect.attr({fill: 'white'});
	for(var i=0; i< node.connectors.length; i++)
		node.connectors[i]
			.toFront()
			.attr({stroke: 'white'});
	
	var childs = TreefindChild(tree, node);
	for(var i=0; i< childs.length; i++) 
	{
		childs[i].rect.attr({fill: 'white'});
		for(var j=0; j< childs[i].connectors.length; j++)
		{
			childs[i].connectors[j]
				.toFront()
				.attr({stroke: 'white'});
		};
	};
};

function NodeSelect(node)
{
	
	if(typeof selectedNode != 'undefined') {
		selectedNode.rect.attr(
				{
					stroke: 'black',
					'stroke-width': 1, 
					dashstyle: 'Solid' 
				}
		);
		
	}
	selectedNode = node;
	var rightContent = document.getElementById('rightContent');
	rightContent.innerHTML = "<h2>"+node.name+"</h2>";
	var select = document.createElement("select");
	select.setAttribute("name", "EASviewselector");
	select.setAttribute("id", "EASviewselector");
	select.style.width = "200px";
	
	rightContent.appendChild(select);
	/* setting an onchange event */
	//selectNode.onchange = function() {dbrOptionChange()};
	
	var option;
	option = document.createElement("option");
	option.setAttribute("value", "1");
	option.innerHTML = "EAS 1";
	select.appendChild(option);
	
	node.rect.attr(
			{
				stroke: 'red',
				'stroke-width': 3, 
				dashstyle: 'ShortDot' 
			}
	);
};

function buildTreeGUI(renderer)
{
	yc = $('#container').height();
	xc = $('#container').width();
	NodeCoo(tree);
	
	for(var l=0; l<tree.NodeList.length; l++) {
		if(tree.NodeList[l] != 'undefined')
			{
			
			tree.NodeList[l].rect = renderer.rect(tree.NodeList[l].x, tree.NodeList[l].y, boxw, boxh, 5).attr({
				fill: 'white',
				stroke: 'black',
				'stroke-width': 1,
				id: tree.NodeList[l].id,
				})
				.on(
					'mouseover', 
					function(e) {
						var node = TreefindById(tree, e.target.id);
						NodeHighlight(node);
					})
				.on(
					'mouseout', 
					function(e) {
						var node = TreefindById(tree, e.target.id);
						NodeUnlight(node);
					})
				.on(
					'click', 
					function(e) {
						var node = TreefindById(tree, e.target.id);
						NodeSelect(node);
					})
				.add();
			if(tree.NodeList[l].img != null)
			{
				tree.NodeList[l].img_render = renderer.image(tree.NodeList[l].img,tree.NodeList[l].x+5, 
						tree.NodeList[l].y+5, boxh-10, boxh-10);
				tree.NodeList[l].img_render.attr(
					{
						id: tree.NodeList[l].id
					}
				)
				.on(
					'mouseover', 
					function(e) {
						var node = TreefindById(tree, e.target.id);
						NodeHighlight(node);
					})
				.on(
					'mouseout', 
					function(e) {
						var node = TreefindById(tree, e.target.id);
						NodeUnlight(node);
					})
				.on(
						'click', 
						function(e) {
							var node = TreefindById(tree, e.target.id);
							NodeSelect(node);
					});
				
				tree.NodeList[l].img_render.add();
				
			};
			tree.NodeList[l].txt = renderer.text(tree.NodeList[l].id,tree.NodeList[l].x + 3*boxh/2,tree.NodeList[l].y + boxh/2+3);
			
			tree.NodeList[l].txt.attr(
					{
						align: 'center',
//						id: tree.NodeList[l].id
					}
			)
			.css(
					{
						fontSize: '12px',
				        color: "#577295"
					}
			)
			.on(
					'mouseover', 
					function(e) {
/*						for(var k in e.target)
							console.log("----> "+k);
						console.log(' ########### '+e.target.textContent);*/
						var node = TreefindById(tree, e.target.textContent);
						NodeHighlight(node);
					}
			)
			.on(
					'mouseout', 
					function(e) {
						var node = TreefindById(tree, e.target.id);
						NodeUnlight(node);
					}
			)
			.on(
					'click', 
					function(e) {
						var node = TreefindById(tree, e.target.id);
						NodeSelect(node);
					}
			);
			tree.NodeList[l].txt.add();
			if(tree.NodeList[l].layer != 1)
			{
				var child = tree.NodeList[l];
				var father = TreefindParent(tree, child);
				if(father.y < child.y)
					{
					tree.NodeList[l].connectors.push(
							renderer.path([ 'M', child.x + boxw/2, child.y, 
					                'L', child.x + boxw/2, father.y + father.conny + boxh,
					                'L', father.x + boxw/2, father.y + father.conny + boxh,
					                'L', father.x + boxw/2, father.y + boxh
					                ]).attr({
						stroke: 'white',
						'stroke-width': 2
						}).add()
					);
				} else {
					tree.NodeList[l].connectors.push(
						renderer.path([ 'M', child.x + boxw/2, child.y + boxh,
					                'L', child.x + boxw/2, (child.y + boxh + father.y)/2,
					                'L', father.x + boxw/2, (child.y + boxh + father.y)/2,
					                'L', father.x + boxw/2, father.y
					                ]).attr({
						stroke: 'white',
						'stroke-width': 2
						}).add()
					);
				}
			};
			
		}
	}	
};

$(function() {
	document.getElementById('leftPanel').style.width = "0%";
	document.getElementById('rightPanel').style.width = "30%";
	document.getElementById('container').style.width = "70%";
	yc = $('#container').height();
	xc = $('#container').width();

	/*
	 * 		Drawing the right panel
	 */		
	var rightContent = document.createElement("div");
	rightContent.id = "rightContent";
	rightContent.setAttribute("class", "RCstyled");
	document.getElementById('rightPanel').appendChild(rightContent);
	
	tree = new Tree();
	tree.addNode(new TreeNode("1.deth0", "deth0"));
	tree.addNode(new TreeNode("1.deth1", "deth1"));
	tree.addNode(new TreeNode("1.deth2", "deth2"));
	tree.addNode(new TreeNode("1.deth3", "deth3"));
	tree.addNode(new TreeNode("0.1", "FE1"));
	tree.addNode(new TreeNode("0.2", "FE2"));
	tree.addNode(new TreeNode("0.3", "FE3"));
	for(var i=0; i<8; i++)
		tree.addNode(new TreeNode("0.2."+i, "FE2 - Core"+i));
	for(var i=0; i<12; i++)
		tree.addNode(new TreeNode("0.1."+i, "FE1 - Core"+i));
	for(var i=0; i<8; i++)
		tree.addNode(new TreeNode("0.3."+i, "FE3 - Core"+i));
	
	for(var i = 0; i<tree.NodeLayers[0].length; i++)
		tree.NodeLayers[0][i].img = "images/network-wired.svg";
	
	for(var i = 0; i<tree.NodeLayers[3].length; i++)
		tree.NodeLayers[3][i].img = "images/cpu.svg";
	
//	console.log("xc= "+ xc +"; yc= "+yc);

	var renderer = new Highcharts.Renderer($('#container')[0],xc,yc);
	
	buildTreeGUI(renderer);
	$(window).resize(function() {
		redrawTree(renderer);
		});
	$(window).trigger('resize'); 
	$(window).on("resize", function() {
		redrawTree(renderer);
		});
	$(document).on('webkitfullscreenchange mozfullscreenchange fullscreenchange',function() { 
		redrawTree(renderer);
		});
	
});