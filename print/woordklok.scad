include <BOSL2/std.scad>
include <BOSL2/screws.scad>

e = 0.01;

// Led strip parameters
l_led = 16.60; // length of one led strip segment, distance between two soldering points (must be exact!)
w_led = 10; // width of led strip (loose fit ~0.5 tolerance)
t_led = 0.6; // thickness of led strip (loose fit ~0.2 tolerance)
t_led_solder = 0.4; // additional thickness of led strip on top of t_led for solder joints

// Led grid parameters
h_grid = 7;
t_grid = 0.8;

t = 1.2;
t_trans = 0.4; // thickness of transparent layer (0 to disable)

text_size = l_led * 0.8;
// Install a stencil font. Copy font in Help > Font List
text_font = "Bunker Stencil";

frame = 9; // extra space around edges

d = 18; // depth of entire module (z direction)

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

txt = [
    "HETISUVIJF",
    "TIENLKWART",
    "OVERSKVOOR",
    "HALFZESEEN",
    "TWEEGIDRIE",
    "VIERBZEVEN",
    "VIJFENEGEN",
    "ACHTIENELF",
    "TWAALFYUUR",
];

leds_x = len(txt[0]);
leds_y = len(txt);

w = leds_x * l_led; // width, excluding frame
h = leds_y * l_led; // height, excluding frame

back_screw = "M3";

cable_position = "right"; // position of usb-c power socket: bottom / right

tol = 0.1;

// nicer circles
$fa = 0.5;
$fs = 0.5;

module power_socket_cutout() {
    cuboid([13.4, t+e, 5.4], chamfer=1, edges="Y");
}

module main() {
    difference() {
        // base plate with letters
        color("orange")
        cuboid([w+frame*2, h+frame*2, t], anchor=TOP);

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
    color("blue")
    down(t)
    cuboid([w+2*frame, h+2*frame, t_trans], anchor=TOP);

    // grid: horizontal rules
    for (i = [0:leds_y])
    fwd(i * l_led - h/2)
    cuboid([w+frame*2, t_grid, h_grid], anchor=BOTTOM);

    // grid: vertical rules
    difference() {
        for (i = [0:leds_x])
        right(i * l_led - w / 2)
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
    color("green")
    difference() {
        x_offset = w/2+frame-t/2+e;
        y_offset = h/2+frame-t/2+e;
        
        union() {
            for (x = [x_offset, -x_offset])
            left(x)
            cuboid([t, h+2*frame, d], anchor=BOTTOM);
                       
            for (y = [y_offset, -y_offset])
            back(y)
            cuboid([w+2*frame, t, d], anchor=BOTTOM);
        }
        
        // USB-C power socket cutout
        up(d/2)
        if (cable_position == "bottom") {
            fwd(y_offset)
            power_socket_cutout();
        } else if (cable_position == "right") {
            left(x_offset)
            fwd(h/2 - frame) // move to bottom of clock, above screw pillar
            zrot(90)
            power_socket_cutout();
        }    
    }

    // stands for screws
    for (x = [(w+frame-t)/2, (w+frame-t)/-2])
    for (y = [(h+frame-t)/2, (h+frame-t)/-2])
    translate([x, y, 0])
    difference() {
        cuboid([frame-t, frame-t, d-t-tol], anchor=BOTTOM);

        screw_hole(back_screw, length=d, anchor=BOTTOM);

        up(d/2)
        nut_trap_side(frame/2, back_screw, spin=x>0 ? 180 : 0);
    }
}

module light_cover() {
    up(h_grid)
    difference() {
        union() {
            // plate
            cuboid([w, h, t], anchor=BOTTOM);
            
            // horizontal line
            fwd(h / 2 + 0.1)
            for (y = [1:leds_x-2]) {
                back(y * l_led)
                cuboid([w-0.2, t_grid*3+2*tol, t_grid], anchor=TOP);
            }
        }
        
        // horizontal slot for grid
        fwd(h / 2 + 0.1)
        for (y = [0:leds_x]) {
            back(y * l_led)
            cuboid([w, t_grid+2*tol, t_grid+e], anchor=TOP);
        }
        
        // vertical slot for grid
        left(w / 2 - t_grid / 2)
        for (x = [0:leds_x]) {
            right(x * l_led)
            cuboid([t_grid+2*tol, h, t_grid+e], anchor=TOP);
        }
        
        // horizontal slot for led strip
        fwd(h / 2 - l_led / 2)
        for (y = [0:leds_x]) {
            back(y * l_led)
            down(e)
            cuboid([w+e, w_led, t_led], anchor=BOTTOM);
        }
    }
}

module back_cover() {
    up(d - t/2)
    difference() {
        union() {
            // plate
            cuboid([w + 2*frame - 2*t - 2*tol, h + 2*frame - 2*t - 2*tol, t]);
            
            // rectangle to push down on light cover
            rect_tube(h=d-h_grid-t-t/2, size=[w/2, h/2], wall=t_grid, anchor=TOP);
        }

        // screw holes
        for (x = [(w+frame-t)/2, (w+frame-t)/-2])
        for (y = [(h+frame-t)/2, (h+frame-t)/-2])
        translate([x, y, 0])
        screw_hole(back_screw, length=t*2, head="flat");
    }
}


main();
light_cover();
//back_cover();
