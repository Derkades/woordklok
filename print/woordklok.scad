include <BOSL2/std.scad>
include <BOSL2/screws.scad>
include <BOSL2/walls.scad>

e = 0.01; // negligible distance to eliminate z-fighting in preview
tol = 0.1;
$slop = tol; // for screws and nuts from BOSL2

// Led strip parameters
l_led = 16.60; // length of one led strip segment, distance between two soldering points (must be exact!)
w_led = 10; // width of led strip (loose fit ~0.5 tolerance)
t_led = 0.6; // thickness of led strip (loose fit ~0.2 tolerance)
t_led_solder = 1; // additional thickness of led strip on top of t_led for solder joints

// Led grid parameters
h_grid = 7;
t_grid = 0.8;

t = 1.2;
t_trans = 0.4; // thickness of transparent layer (0 to disable)

text_size = l_led * 0.8;
// Install a stencil font. Copy font in Help > Font List
text_font = "Bunker Stencil";

frame = 9; // extra space around edges

d = 20; // depth of entire module (z direction)
d_inner = d - 2*t - tol; // inner depth, excluding back panel

//txt = [
//    "HETUISEVIJF",
//    "TIENHRAVOOR",
//    "OVERAOKWART",
//    "HALFLBAOVER",
//    "VOORLIKDEEN",
//    "TWEEONJDRIE",
//    "VIERVIJFZES",
//    "ZEVENENEGEN",
//    "ACHTIENSELF",
//    "TWAALFBFUUR",
//];

// NL 10x10
txt = [
    "ODHETUISIB",
    "VIJFATIENE",
    "KWARTFOVER",
    "VOOREHALFL",
    "JEENRTWEEQ",
    "DRIEHVIERI",
    "VIJFNZEVEN",
    "SNEGENXZES",
    "ACHTIENELF",
    "TWAALFMUUR",
];

leds_x = len(txt[0]);
leds_y = len(txt);

w = leds_x * l_led; // width, excluding frame
h = leds_y * l_led; // height, excluding frame

back_screw = "M3";
back_screw_reinforcement_d = 6;

power_socket_position = "right"; // position of usb-c power socket: bottom / right
power_socket_offset = t + 1.95 + 5.4/2; // distance between from back and power socket center

wall_mount = "screw"; // "string" / "screw"
d_wall_mount_screw = 4;
d_wall_mount_screw_head = 8;

// nicer circles
$fa = 0.5;
$fs = 0.5;

rounding = t*2;

module power_socket_cutout() {
    cuboid([13.4, t+e, 5.4], chamfer=1, edges="Y");
}

module main() {
    difference() {
        // base plate with letters
        color("orange")
        cuboid([w+frame*2, h+frame*2, t], anchor=TOP, rounding=rounding, edges="Z");

        // letters
        up(e)
        for (i = [0:leds_y])
        fwd(i *l_led - h/2 + l_led/2 + text_size/2)
        mirror([1, 0, 0])
        for (j = [0:leds_x-1])
        right(j * l_led - h/2)
        text3d(txt[i][j], h=t+2*e, anchor=TOP, size=text_size, font=text_font);
    }

    // transparent base plate
    // slightly larger so it can be selected separately in the slicer (e.g. to be excluded from fuzzy skin)
    color("blue")
    down(t)
    cuboid([w+2*frame+e, h+2*frame+e, t_trans], anchor=TOP, rounding=rounding, edges="Z");

    // grid: horizontal rules
    for (i = [0:leds_y])
    fwd(i * l_led - h/2)
    cuboid([w+frame*2, t_grid, h_grid], anchor=BOTTOM);

    // grid: vertical rules
    difference() {
        for (i = [0:leds_x])
        right(i * l_led - w/2)
        cuboid([t_grid, h+frame*2, h_grid], anchor=BOTTOM);

        // slots for led strip solder joints
        up(h_grid + e)
        for (x = [-w/2, w/2])
        right(x)
        for (y = [1:leds_x-1])
        fwd(y*l_led - h/2 - l_led/2)
        cuboid([t_grid+e, w_led, t_led_solder], anchor=TOP);
    }

    // outer frame
    *color("green")
    difference() {
        down(e)
        rect_tube(d, size=[w+frame*2, h+frame*2], wall=t,rounding=rounding);

        // USB-C power socket cutout
        up(d - power_socket_offset)
        if (power_socket_position == "bottom") {
            fwd(h/2+frame-t/2)
            power_socket_cutout();
        } else if (power_socket_position == "right") {
            left(w/2+frame-t/2)
            fwd(h/2 - frame) // move to bottom of clock, above screw pillar
            zrot(90)
            power_socket_cutout();
        }
    }

    // stands for screws
    for (x = [(w+frame-t)/2, (w+frame-t)/-2])
    for (y = [(h+frame-t)/2, (h+frame-t)/-2])
    translate([x, y, d_inner])
    difference() {
        cuboid([frame-t, frame-t, d_inner], anchor=TOP);

        screw_hole(back_screw, length=d, anchor=TOP);

        down(2*t)
        nut_trap_side(frame/2, back_screw, spin=x>0 ? 180 : 0, anchor=TOP);
    }
    
    // trim to support back cover    
    up(d_inner)
    difference() {
        rect_tube(size=[w+2*frame-2*t, h+2*frame-2*t], h=t*2, wall=t, anchor=TOP);
        
        down(t)
        cuboid([w+2*frame-2*t, h+2*frame-2*t, t], chamfer=t, edges=TOP, anchor=TOP);
    }
}

module light_cover() {
    up(h_grid)
    difference() {
        union() {
            // plate
            color("teal")
            cuboid([w, h, t], anchor=BOTTOM, chamfer=tol, edges="Z");

            // horizontal line
            fwd(h / 2 + 0.1)
            for (y = [1:leds_x-2]) {
                back(y * l_led)
                cuboid([w-0.2, t_grid*3+2*tol, t_grid], anchor=TOP);
            }
        }

        // horizontal slot for grid
        for (y = [0:leds_x])
        back(y * l_led - h/2)
        cuboid([w, t_grid+2*tol, t_grid+e], anchor=TOP);

        // vertical slot for grid
        for (x = [0:leds_x])
        right(x * l_led - w/2)
        cuboid([t_grid+2*tol, h, t_grid+e], anchor=TOP);

        // horizontal slot for led strip
        down(e)
        for (y = [0:leds_x])
        back(y*l_led - h/2 - l_led/2)
        cuboid([w+e, w_led, t_led], anchor=BOTTOM);
    }
}

module back_cover() {
    plate_w = w + 2*frame - 2*t - 2*tol;
    plate_h = h + 2*frame - 2*t - 2*tol;
    
    
    up(d - t/2)
    difference() {
        union() {
            // plate            
            cuboid([plate_w, plate_h, t], rounding=rounding/2, edges="Z");
            
            // for stiffness
            plate2d = rect([plate_w, plate_h], rounding=rounding/2);
            down(t)
            hex_panel(plate2d, height=t, strut=t_grid, spacing=10);
            
            // screw hole reinforcement
            for (x = [(w+frame-t-tol)/2, (w+frame-t-tol)/-2])
            for (y = [(h+frame-t-tol)/2, (h+frame-t-tol)/-2])
            translate([x, y, -t])
            cuboid([frame-t-tol, frame-t-tol, t], rounding=rounding/2, edges="Z");
            
            // rectangle to push down on light cover
            rect_tube(h=d-h_grid-t-t/2, size=[w/2, h/2], wall=t_grid, anchor=TOP, chamfer=5);
        }

        // screw holes
        for (x = [(w+frame-t)/2, (w+frame-t)/-2])
        for (y = [(h+frame-t)/2, (h+frame-t)/-2])
        translate([x, y, t/2])
        screw_hole(back_screw, length=t*3, head="flat", anchor=TOP);
        
        // wall mount
        down(t/2)
        back(h/3-t)
        if (wall_mount == "string") {
            for (x = [-w/3, w/3])
            right(x)
            cyl(d=3, h=t*2+e);
        } else if (wall_mount == "screw") {
            cyl(d=d_wall_mount_screw_head, h=2*t+e);
            back(d_wall_mount_screw_head/2)
            cuboid([d_wall_mount_screw, d_wall_mount_screw_head, 2*t+e], rounding=d_wall_mount_screw/2, edges="Z");
        }
    }
}

main();
//light_cover();
back_cover();
