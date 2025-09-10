include <BOSL2/std.scad>

e = 0.01;

x_led = 16.7;
z_led = 10;

t = 2;
t_grid = 0.8;
t_trans = 0; // thickness of transparent layer (0 to disable)
t_led = 2;

text_size = x_led * 0.7;
// Install a stencil font. Copy font in Help > Font List
text_font = "Stencil Gothic:style=Regular";

frame = 10; // extra space around edges

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
    "VIERXECVIJF",
    "ZESDIOZEVEN",
    "ACHTQNNEGEN",
    "TIENDUZVELF",
    "TWAALFCWUUR",  
];

leds_x = len(txt[0]);
leds_y = len(txt);

w = leds_x * x_led; // width, excluding frame
h = leds_y * x_led; // height, excluding frame

render_text = true; // set to false to speed up rendering

// nicer circles
$fa = 0.5;
$fs = 0.5;

difference() {
    // base plate with letters
    cuboid([w+frame*2, h+frame*2, t], anchor=TOP);

    // letters
    if (render_text) {
        right(w / 2 - x_led/2)
        back(h / 2 - x_led)
        for (i = [0:leds_y]) {
            fwd(i * x_led - (x_led - text_size)/2)
            
            mirror([1, 0, 0])
            for (j = [0:leds_x-1]) {
                right(j * x_led)
                text3d(txt[i][j], h=t, anchor=TOP, size=text_size, font=text_font);
            }
        }
    }
}

// transparent base plate
down(t)
#cuboid([w, h, t_trans], anchor=TOP);

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
        cuboid([t_grid, w, z_led], anchor=BOTTOM+LEFT);
    }
    
    // make room for led strips
    back(h / 2 + t_grid)
    for (i = [1:leds_y]) {
        fwd(i * (x_led - t_grid / leds_y))
        cuboid([w, t_grid, z_led+e], anchor=BOTTOM+BACK);
    }
}

// outer frame
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