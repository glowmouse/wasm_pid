draw_arm = 0;
draw_base = 1;
draw_arm_demo = 0;

module box( length, width, height )
{
    linear_extrude( height ) polygon([
        [ -length/2, -width/2 ],
        [  length/2, -width/2 ],
        [  length/2,  width/2 ],
        [ -length/2,  width/2 ]] );
}

x_arm_pivot_0 = 0;
y_arm_pivot_0 = 0;
z_arm_pivot_0 = 15;
d_arm_pivot_0 = 11;

x_arm_pivot_1 = 0;
y_arm_pivot_1 = 0;
z_arm_pivot_1 = 15;
d_arm_pivot_1 = 11;

x_arm_pivot_2 = 0;
y_arm_pivot_2 = 0;
z_arm_pivot_2 = 15;
d_arm_pivot_2 = 0;

module arm_attach( x_arm_pivot, y_arm_pivot, z_arm_pivot, d_arm_pivot, warp7 )
{
    z_gap_top = 2;
    z_gap_bot = 7;
    z_gap_arm = 2;

    translate( [ 0,  -3, z_gap_bot ] )
    minkowski() {
        box( 3, .01, z_arm_pivot+z_gap_top - z_gap_bot );
        sphere(r=1, $fn=20);
    }
    translate( [ 0,  3, z_gap_bot ] )
    minkowski() {
        box( 3, .01, z_arm_pivot+z_gap_top - z_gap_bot );
       sphere(r=1, $fn=20);        
    }
    translate( [ 0, y_arm_pivot - d_arm_pivot/2,        z_arm_pivot ] ) {
        rotate(-90, [1,0,0] )
        cylinder( r = 1, h=d_arm_pivot, $fn=15 );
    }
    translate([ 0, 0, -z_gap_arm ])
    minkowski() {
        box( 3, 2, z_arm_pivot - z_gap_top - z_gap_arm );
        sphere(r=1, $fn=20);
    }
    if ( 1 ) {
        translate( [-0.8, -y_arm_pivot +3.6, z_arm_pivot-8.2] )
        rotate(90,[0,1,0])
        rotate(90,[1,0,0])
        rotate(180,[0,1,0])
        linear_extrude(height=1) text("WARP7",size=1.4);
    }
}

module arm()
{
    translate([0,0,-3])    
    cylinder( r = 12, h = 5, $fn=80 );

    arm_attach( x_arm_pivot_0, y_arm_pivot_0, z_arm_pivot_0, d_arm_pivot_0 );
    translate( [0, 0, z_arm_pivot_0 ] )
    rotate(-45, [0, 1, 0 ] )
    {
    arm_attach( x_arm_pivot_1, y_arm_pivot_1, z_arm_pivot_1, d_arm_pivot_1 );
//    translate( [0, 0, z_arm_pivot_1 ] )
//    rotate(-135, [0, 1, 0 ] )        
//    arm_attach( x_arm_pivot_2, y_arm_pivot_2, z_arm_pivot_2, d_arm_pivot_2 );        
    }
}

if ( draw_base ) {
    rotate(180,[0,0,1])
    arm();
}

x_join = sin(45) * z_arm_pivot_1;
z_join = z_arm_pivot_0 + cos(45) * z_arm_pivot_1;

if ( draw_arm_demo ) {
translate([ x_join, 0, z_join ] ) 
rotate(170, [0,1,0])
arm_attach( x_arm_pivot_2, y_arm_pivot_2, z_arm_pivot_2, d_arm_pivot_2 );    
}

if ( draw_arm ) {
    rotate(180,[0,0,1])
    arm_attach( x_arm_pivot_2, y_arm_pivot_2, z_arm_pivot_2,    d_arm_pivot_2 );       
}

echo( x_join );
echo( z_join );
