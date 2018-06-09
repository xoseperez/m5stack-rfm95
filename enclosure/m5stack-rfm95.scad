// -------------------------------------------------
// Settings
// -------------------------------------------------

$fn = 50;

// -------------------------------------------------
// Configuration
// -------------------------------------------------

outer_size = 54.0;
outer_corner = 5.0;
wall_width = 1.7;
wall_height = 10.0;
tab_width = 0.7;
tab_height = 1.0;
hole_height = 3.0;
top_height = 4.0;

// -------------------------------------------------

inner_size = outer_size - 2 * wall_width;
inner_corner_diameter = outer_corner - wall_width;

// -------------------------------------------------
// Modules
// -------------------------------------------------

module wall(outer, width, corner) {

    inner = outer - 2 * width;
    inner_corner = corner - width;

    difference() {

        minkowski() {
            square(outer - 2 * corner, true);
            circle(corner);
        }


        minkowski() {
            square(inner - 2 * inner_corner, true);
            circle(inner_corner);
        }

    }

}

module walls() {
    wall(outer_size, wall_width, outer_corner);
}

module tabs() {
    outer = outer_size + (tab_width - wall_width) * 2;
    corner = outer_corner + (tab_width - wall_width);
    difference() {
        wall(outer, tab_width, corner);
        square([32,outer_size], true);
        square([outer_size,39], true);
    }
}

module hole() {
    translate([18,22]) {
        difference() {
            union() {
                circle(3.7/2);
                translate([-3.7/2,0]) square([3.7,5]);
            }
            circle(2.0/2);
        }
    }
}

module holes() {
    hole();
    mirror([1,0,0]) hole();
    mirror([0,1,0]) {
        hole();
        mirror([1,0,0]) hole();
    }
}

module antenna() {
    rotate([90,0,0]) linear_extrude(wall_width*2) circle(7/2);
}

module screw() {
    linear_extrude(top_height) circle(3/2);
    linear_extrude(top_height/2) circle(6/2);
}

module top() {

    minkowski() {
        square(outer_size - 2 * outer_corner, true);
        circle(outer_corner);
    }

}

module bottom_layer() {
    union() {
        translate([0,0,top_height]) {
            difference() {
                linear_extrude(wall_height) walls();
                translate([0,outer_size/2,height-6]) antenna();
            }
            translate([0,0,wall_height-hole_height]) {
                linear_extrude(hole_height) holes();
            }
            translate([0,0,wall_height]) {
                linear_extrude(tab_height) tabs();
            }
        }
        difference() {
            linear_extrude(top_height) top();
            translate([22,-22]) screw();
            translate([-22,-22]) screw();
            translate([-22,22]) screw();
            translate([22,22]) screw();
        }
    }
}

module middle_layer() {
    height = 12.5;
    difference() {
        union() {
            difference() {
                linear_extrude(height) walls();
                translate([0,outer_size/2,height-6]) antenna();
            }
            translate([0,0,height-hole_height]) {
                linear_extrude(hole_height) holes();
            }
            translate([0,0,height]) {
                linear_extrude(tab_height) tabs();
            }
        }
        linear_extrude(tab_height) tabs();
    }
}

// -------------------------------------------------
// Entry points
// -------------------------------------------------

//bottom_layer();
middle_layer();
