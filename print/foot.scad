include <BOSL2/std.scad>
include <BOSL2/screws.scad>

$fa = 0.5;
$fs = 0.5;
$slop = 0.1;

base_w = 70;
tw = 10;
th = 5;
clock_w = 22;
h = 80;
attach_h = 40;
screw_dist = 30;

cuboid([base_w, th, tw], chamfer=th/2, edges=[BACK+RIGHT]);

hull() { 
    back(th/2)
    left(base_w/2)
    #cuboid([th, 0.1, tw], anchor=LEFT+FRONT);
    
    back(h)
    left(clock_w/2)
    #cuboid([th, 0.1, tw], anchor=RIGHT+BACK);
}

left(clock_w/2)
back(h)
difference() {
    cuboid([th, attach_h, tw], anchor=RIGHT+FRONT);
    
    for (b = [attach_h/2-screw_dist/2, attach_h/2+screw_dist/2])
    back(b)
    left()
    yrot(-90)
    screw_hole("M3", l=th, anchor=BOTTOM, head="flat");
}