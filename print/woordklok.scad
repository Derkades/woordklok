include <BOSL2/std.scad>
include <BOSL2/screws.scad>

e = 0.01;

x_led = 16.7;
z_led = 10;

t = 1.4;
t_grid = 0.8;
t_trans = 0.8; // thickness of transparent layer (0 to disable)
t_led = 1.4;

text_size = x_led * 0.7;
// Install a stencil font. Copy font in Help > Font List
text_font = "Bunker Stencil:style=Regular";

frame = 9; // extra space around edges

d = 20; // depth of entire module (z direction)

d_cable = 5; // cable cutout diameter
cable_side = false;

txt = [
    "HETUISEVIJF",
    "TIENHRAVOOR",
    "OVERAOKWART",
    "HALFLBAOVER",
    "VOORLIKDEEN",
    "TWEEONJDRIE",
    "VIERVIJFZES",
    "ZEVENONEGEN",
    "ACHTIENSELF",
    "TWAALFBFUUR",
];

leds_x = len(txt[0]);
leds_y = len(txt);

w = leds_x * x_led; // width, excluding frame
h = leds_y * x_led; // height, excluding frame

render_text = true; // set to false to speed up rendering

back_screw = "M3";

// nicer circles
$fa = 0.5;
$fs = 0.5;

module main() {
    t_base = t-t_trans;
    difference() {
        // base plate with letters
        cuboid([w+frame*2, h+frame*2, t_base], anchor=TOP);

        // letters
        if (render_text) {
            right(w / 2 - x_led/2)
            back(h / 2 - x_led)
            for (i = [0:leds_y]) {
                up(0.001)
                fwd(i * x_led-text_size/2+t_grid*2)
                
                mirror([1, 0, 0])
                for (j = [0:leds_x-1]) {
                    right(j * x_led)
                    text3d(txt[i][j], h=t_base+0.002, anchor=TOP, size=text_size, font=text_font);
                }
            }
        }
    }

    // transparent base plate
    recolor("blue")
    down(t_base)
    cuboid([w+2*frame, h+2*frame, t_trans], anchor=TOP);

    // grid: horizontal rules
    back(h / 2)
    for (i = [0:leds_y]) {
        fwd(i * (x_led - t_grid / leds_y))
        cuboid([w, t_grid, z_led], anchor=BOTTOM+BACK);
    }

    // grid: vertical rules
    difference() {
        left(w / 2)
        for (i = [0:leds_x]) {
            right(i * (x_led - t_grid / leds_x))
            cuboid([t_grid, h, z_led], anchor=BOTTOM+LEFT);
        }
        
        // make room for led strips
        back(h / 2)
        for (i = [1:leds_y]) {
            fwd(i * (x_led - t_grid / leds_y))
            cuboid([h, t_led, z_led+e], anchor=BOTTOM+FRONT);
        }
    }

    // outer frame
    recolor("green")
    difference() {
        union() {
            back(h/2+frame)
            cuboid([w+2*frame, t, d], anchor=BOTTOM+BACK);
            fwd(h/2+frame)
            cuboid([w+2*frame, t, d], anchor=BOTTOM+FRONT);

            left(w/2+frame)
            cuboid([t, h+2*frame, d], anchor=BOTTOM+LEFT);
            right(w/2+frame)
            cuboid([t, h+2*frame, d], anchor=BOTTOM+RIGHT);
        }
        
        // cable cutout
        up((d+t+t_trans)/2)
        if (cable_side) {
            fwd(h/2)
            right(w/2+frame)
            yrot(90)
            #cyl(d=d_cable, h=t, anchor=TOP+LEFT);
        } else {
            fwd(h/2+frame)
            xrot(90)
            cyl(d=d_cable, h=t, anchor=TOP+BACK);
        }
    }
    
    // stands for screws
    for (x = [(w+frame)/2, (w+frame)/-2]) {
        for (y = [(h+frame)/2, (h+frame)/-2]) {
            translate([x, y, 0])
            difference() {
                cuboid([frame, frame, d-t_grid], anchor=BOTTOM);
                
                screw_hole(back_screw, length=d, anchor=BOTTOM);
                
                up(d/2)
                nut_trap_side(frame/2, back_screw, spin=x>0 ? 180 : 0);
            }
        }
    } 
}

module light_cover() {
    up(z_led)
    cuboid([w, h, t_grid], anchor=BOTTOM) {
        difference() {
            position(BOTTOM) {
                left(w / 2 - t_grid / 2)
                for (x = [1:leds_x-1]) {
                    right(x * x_led)
                    cuboid([t_grid*3+0.2, h, t_grid], anchor=TOP);
                }
               
                fwd(h / 2 + 0.1)
                for (y = [1:leds_x-1]) {
                    back(y * x_led)
                    cuboid([w, t_grid, t_grid], anchor=TOP+BACK);
                }
            }
            
            position(BOTTOM) {
                left(w / 2 - t_grid / 2)
                for (x = [0:leds_x]) {
                    right(x * x_led)
                    cuboid([t_grid+0.2, h, t_grid], anchor=TOP);
                }
                
                fwd(h / 2 + 0.1)
                for (y = [0:leds_x]) {
                    back(y * x_led)
                    cuboid([w, t_grid+t_led, t_grid], anchor=TOP+FRONT);
                }
            }
        }
    }
}

module back_cover() {
    up(d) {
        difference() {
            union() {
                cuboid([w+2*frame, h+2*frame, t], anchor=BOTTOM);
                
                rect_tube(h=t_grid, size=[w+2*frame-t*2-0.2, h+2*frame-t*2-0.2], wall=t_grid, anchor=TOP);
                
                tube(h=d-z_led-t_grid, od=min(h, w)*0.7, wall=t_grid, anchor=TOP);
                
                for (x = [(w+frame)/2, (w+frame)/-2]) {
                    for (y = [(h+frame)/2, (h+frame)/-2]) {
                        translate([x, y, 0])
                        cuboid([frame-t*2-0.2, frame-t*2-0.2, t_grid], anchor=TOP);
                    }
                }
            }
            
            for (x = [(w+frame)/2, (w+frame)/-2]) {
                for (y = [(h+frame)/2, (h+frame)/-2]) {
                    translate([x, y, 0])
                    screw_hole(back_screw, length=t*2, head="flat");
                }
            }
        }
      
    }
}


main();
//light_cover();
//back_cover();