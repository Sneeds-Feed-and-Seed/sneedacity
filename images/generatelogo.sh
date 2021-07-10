#! /bin/bash

names="gnome-mime-application-x-sneedacity-project.xpm;48x48 sneedacity.png;16x16 sneedacity.png;22x22 sneedacity.png;24x24  sneedacity.png;32x32  sneedacity.png;48x48 sneedacity16.xpm;16x16 sneedacity32.xpm;32x32 sneedacity.xpm;48x48 _default_instrument.xpm;48x48 SneedacityLogo.png;60x60 SneedacityLogo.xpm;200x200 SneedacityLogo48x48.xpm;48x48 SneedacityLogoAlpha.xpm;48x48"


for name in $names  
do   
	#
	tempsize=$(echo $name | sed 's/.*; *//')
	tempname=$(echo $name | sed 's#;.*$##g')
	mkdir $tempsize
	cd $tempsize
	convert ../logo.png -resize $tempsize $tempname
	cd ..
done
