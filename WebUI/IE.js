/**
 * The IE class
 */


function IE(name_tmp, renderer, layout, master) {
	this.renderer = renderer;
	this.name = name_tmp;
	this.layout = layout; //it can be "horizontal" or "vertical".
	this.masterElement = master; //the master Element is the one drawn at the top of the container, and it has no connections 
	
	this.form = new Array();
	this.IEx = 25;
	this.IEy = 25;
	this.IEboxwidth = 750;
	this.IEboxhigh = 100;
	this.oldIEx = 0;
	this.boxattr1 = {
			fill : {
				radialGradient: { cx: 0, cy: 0.3, r: 1 },
		 		stops: [ 
		 	         [0, 'white'], 
		 	         [1, '#BBBBBB'] ]
			},
			zIndex : 3,
//			opacity: 0
		};
	this.IEicon= 'mlx.png';
	this.IEicon_xoffset=425;
	this.IEicon_yoffset=25;
	this.IEtxt_xoffset=60;
	this.IEtxt_yoffset=60;
	this.IE_iconwidth=300;
	this.IE_iconh=50;
	this.IE1boxy=125; 
	
	this.txtattr={
	        align: 'center',
	        zIndex: 5,
	        padding: 0,
	        opacity: 0
	    };
	this.txtcss={
	        fontSize: '50px',
	        color: "#577295"
	    }; 
	this.animate_Appear_attr = { opacity: 1 };
	this.animate_Disppear_attr = { opacity: 0 };
	this.animatetime={ duration: 1000 }; 
	this.lURL="index.html";

}


IE.prototype.draw = function()
{
	this.oldIEx=this.IEx;
	this.oldwidth=this.IEboxwidth;
	this.form.push( this.renderer.rect(this.IEx, this.IEy, this.IEboxwidth, this.IEboxhigh, 10)
			.attr(this.boxattr1)
			);
/*			.on('mouseover', function() {this.form[0].attr({fill :  'red'});})
			.on('mouseout', function() {this.form.attr(boxattr1)
										.attr({opacity : 1});
		}))*/;
	
	
	if ( this.layout == "vertical")
		{
			this.form.push( this.renderer.image(this.IEicon,this.IEx+(this.IEboxwidth-this.IE_iconwidth)/2+this.IEicon_xoffset, 
					this.IEy + this.IEicon_yoffset, this.IE_iconwidth, this.IE_iconh)
				.attr({	zIndex : 4 	}));
		} else {
			
			this.form.push( this.renderer.image(this.IEicon,this.IEx+this.IEicon_xoffset, 
					this.IEy + this.IEicon_yoffset, this.IE_iconwidth, this.IE_iconh)
				.attr({	zIndex : 4 	}));
		};
		this.form.push( this.renderer.text(this.name,this.IEx+this.IEtxt_xoffset,this.IEy+this.IEtxt_yoffset)
				.attr(this.txtattr)
				.css(this.txtcss));
	if ( ! this.masterElement )
		{
			for(var i=0; i< this.no_conn; i++) {
			this.form.push(this.renderer.path([ 
			                                    'M', this.IEx+this.IEboxwidth/2 + 10*(i-this.no_conn/2), this.IEy, 
			                                    'L', this.IEx+this.IEboxwidth/2+ 10*(i-this.no_conn/2), this.IE1boxy 
			                                    ])
				.attr({
					'stroke-width' : 5,
					stroke : 'white',
					opacity : 0
				}));
			}
		};
		
//	this.form.AddLink(this.lURL);
	
	this.form.animateRender(this.animate_Appear_attr, this.animatetime);
	this.form.addToRender();
};

IE.prototype.appear = function()
{
	this.form.animateRender(this.animate_Appear_attr, this.animatetime);
};

IE.prototype.disappear = function()
{
	this.form.animateRender(this.animate_Disppear_attr, this.animatetime);
};

IE.prototype.redraw = function()
{
	if ( ! this.masterElement )
	{
		var tmp_xoff = this.IEx+this.IEboxwidth/2-this.oldIEx-this.oldwidth/2;
//		console.log("tmp_xoff:" + tmp_xoff + " new_x:"+this.IEx+" new_w:"+this.IEboxwidth+" old_x:"+this.oldIEx+" old_w:"+this.oldwidth );
	};
	this.form[0].attr(
			{
				x: this.IEx,
				y: this.IEy,
				width :  this.IEboxwidth,
				height : this.IEboxhigh
			}
		);
	if ( this.layout == "vertical")
	{
		this.form[1].attr(
				{
					x: this.IEx+(this.IEboxwidth-this.IE_iconwidth)/2+this.IEicon_xoffset,
					y: this.IEy + this.IEicon_yoffset,
					width: this.IE_iconwidth,
					height: this.IE_iconh
				}
			);
	} else {
		this.form[1].attr(
				{
					x: this.IEx+this.IEicon_xoffset, 
					y: this.IEy + this.IEicon_yoffset,
					width: this.IE_iconwidth,
					height: this.IE_iconh
				}
			);
	};
	this.form[2].attr(
			{
				x: this.IEx+this.IEtxt_xoffset,
				y: this.IEy+this.IEtxt_yoffset
			}
		);
	if ( ! this.masterElement )
	{
		for(var i=0; i< this.no_conn; i++) {
			this.form[3+i].attr({
				translateX: tmp_xoff
			});
		};
	};
};