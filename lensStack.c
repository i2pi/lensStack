#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
** All dimensions in millimeters
*/


typedef enum {
    lens,
    aperture
} elementTypeE;

typedef struct elementT {
    elementTypeE    type;
    union {
        struct {
            double  focal_length;
            double  diameter;
            double  width;
        } lens;
        struct {
            double  diameter;
        } aperture;
    } e;
} elementT;

typedef struct elementGroupT {
    elementT    element[64];
    double      position[64];   // measured from the focal plane
    int         elements;
} elementGroupT;

int init_element (elementT *e, elementTypeE type) {
    e->type = type;
    
    if (type == lens) {
        e->e.lens.focal_length = FP_NAN;
        e->e.lens.diameter = FP_NAN;
        e->e.lens.width = FP_NAN;
        return(0);
    };

    if (type == aperture) {
        e->e.aperture.diameter = FP_NAN;
        return (0);
    }

    printf ("Unknown lens type [%d]\n", type);
    exit (-1);
}

int init_element_group (elementGroupT *eg, int elements) {
    int i;

    eg->elements = elements;

    for (i=0; i<elements; i++) {
        init_element(&eg->element[i], lens);
        eg->position[i] = FP_NAN;
    } 
    return (0);
}

double group_focal_length (elementGroupT *eg) {

    /*
    ** TODO: This doesn't really take in to account the thickness of the lenses..
    */

    double  f1, f2;
    double  d, p, w;
    int     i;

    // skip any apertures
    i = 0;
    while (eg->element[i].type != lens && i < eg->elements) i++;
    if (i >= eg->elements) {
        printf ("No lenses found in group.\n");
        exit (-1);
    }

    f1 = eg->element[i].e.lens.focal_length;
    w = eg->element[i].e.lens.width;
    p = eg->position[i];

    for (i=i+1; i<eg->elements; i++) if (eg->element[i].type == lens) {
        f2 = eg->element[i].e.lens.focal_length;
        d = p - eg->position[i];

        if (d < eg->element[i].e.lens.width*0.5 + w*0.5) {
         printf ("Lenses %d & %d are too close together\n", i, i-1);
         exit (-1);
        }

        f1 = (f1 * f2) / (f1 + f2 - d);

        p = eg->position[i];
        w = eg->element[i].e.lens.width;
    }

    return (f1);
}

double  restrict_ray_height (double y, double d) {
    if (fabs(y) > d*0.5) {
        return(d * 0.5 * ((y>0) - (y<0)));
    }
    return(y);
}

void    group_thin_lens (elementGroupT *eg, double *efl, double *bfl) {
    double  y, y1;
    double  u, p;

    int i;

    // skip any apertures
    i = 0;
    while (eg->element[i].type != lens && i < eg->elements) i++;
    if (i >= eg->elements) {
        printf ("No lenses found in group.\n");
        exit (-1);
    }
    
    y1 = eg->element[i].e.lens.diameter * 0.5;
    y = y1;
    u = -y / eg->element[i].e.lens.focal_length;
    p = eg->position[i];

    for (i=i+1; i<eg->elements; i++) {
        while (eg->element[i].type != lens && i < eg->elements) {
            // Check that it fits in the aperture / lens
            if (eg->element[i].type == aperture) y = restrict_ray_height (y, eg->element[i].e.aperture.diameter); 
            i++;
        }

        y += u * (p - eg->position[i]);
        u = u - y / eg->element[i].e.lens.focal_length;
        p = eg->position[i];
    }

    *efl = -y1 / u;
    *bfl = -y / u;
}



int main (int argc, char **argv) {

    elementGroupT   eg;
    double efl, bfl;

    init_element_group(&eg, 5);

    eg.element[0].e.lens.focal_length = 200;
    eg.element[0].e.lens.diameter = 50;
    eg.element[0].e.lens.width = 5;
    eg.position[0] = 100;

    eg.element[1].e.lens.focal_length = 200;
    eg.element[1].e.lens.diameter = 50;
    eg.element[1].e.lens.width = 5;
    eg.position[1] = 95;

    eg.element[2].e.lens.focal_length = 200;
    eg.element[2].e.lens.diameter = 50;
    eg.element[2].e.lens.width = 5;
    eg.position[2] = 90;

    eg.element[3].type = aperture;
    eg.element[3].e.aperture.diameter = 40;
    eg.position[3] = 87;

    eg.element[4].e.lens.focal_length = 200;
    eg.element[4].e.lens.diameter = 50;
    eg.element[4].e.lens.width = 5;
    eg.position[4] = 84;

    efl = group_focal_length (&eg);
    printf ("Group focal length = %6.4f\n", efl); 

    group_thin_lens (&eg, &efl, &bfl);
    printf ("Group thin lens: elf=%6.4f, bfl=%6.4f\n", efl, bfl); 
    
}
