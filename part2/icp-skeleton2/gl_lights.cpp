glm::vec3 color_red(1.0f, 0.0f, 0.0f);
glm::vec3 color_green(0.0f, 1.0f, 0.0f);
glm::vec3 color_blue(0.0f, 0.0f, 1.0f);
glm::vec3 color_white(1.0f, 1.0f, 1.0f);
glm::vec3 color_black(0.0f, 0.0f, 0.0f);

GLfloat color_grey[]={0.7, 0.7, 0.7, 1.0};
GLfloat color_white[]={1.0, 1.0, 1.0, 1.0};
GLfloat color_R[]={1.0, 0.0, 0.0, 1.0};
GLfloat color_G[]={0.0, 1.0, 0.0, 1.0};
GLfloat color_B[]={0.0, 0.0, 1.0, 1.0};

GLfloat color_Ra[]={1.0, 0.0, 0.0, 0.3};
GLfloat color_Ga[]={0.0, 1.0, 0.0, 0.3};
GLfloat color_Ba[]={0.0, 0.0, 1.0, 0.3};

void DrawAll(void)
{
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );       // vymazani dvou bufferu

...

    //non-transparent object, final colour through Phong lighting model
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color_grey );

    // ... draw3 objects ... 

	//semi-transparent object, colour through Phong model
	glEnable( GL_BLEND );
    glBegin(GL_TRIANGLES);              
        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color_Ra );
        glVertex3i(-25, -14, 0 );            
        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color_Ga );
        glVertex3i( 25, -14, 0 );          
        //NOTICE, that used material is W/O transparency, 
        //transparency is interpolated same way as colour
        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color_B ); 
        glVertex3i(  0,  29, 0 );           
    glEnd();
  glDisable( GL_BLEND );

...

}

void init( void )
{
    glClearColor(0.0,0.0,0.0,0.0);

    glEnable( GL_DEPTH_TEST ); 
    glEnable( GL_POLYGON_SMOOTH ); //antialiasing of faces
    glEnable( GL_LINE_SMOOTH );    //antialiasing of lines

    //lighting model setup
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, color_grey );  //colour of default ambient light
    glShadeModel( GL_SMOOTH );   //Gouraud shading
    glEnable( GL_NORMALIZE );    //normalisation of EVERYTHING! SLOW! (but safe...) 
    glEnable( GL_LIGHT0 );       
    glEnable( GL_LIGHTING );     

    //transparency blending function
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

int main( int argc, char * argv[])
{

    glfw....

    init();

    while(true)
    {
      ...
      draw();
      ...
    }

    return EXIT_SUCCESS;
}
