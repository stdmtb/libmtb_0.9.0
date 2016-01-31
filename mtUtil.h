#pragma once
#include <fstream>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/wrapper.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef std::function<float (float)> EaseFunc;

namespace mt {
    
    fs::path assetDir;
    
    long getSeed(){
        time_t curr;
        tm local;
        time(&curr);
        local =*(localtime(&curr));
        //return local.tm_gmtoff;
        return local.tm_sec + local.tm_min;
    }
    
    string getTimeStamp(){        
        time_t curr;
        tm local;
        time(&curr);
        local =*(localtime(&curr));
        int month = local.tm_mon;
        int day = local.tm_mday;
        int hours = local.tm_hour;
        int min = local.tm_min;
        int sec = local.tm_sec;
        
        char stamp[16];
        sprintf(stamp, "%02d%02d_%02d%02d_%02d", month+1, day, hours, min, sec);
        return string(stamp);
    }
    
    string getTimeStampU(){
        return toString( time(NULL) );
    }
    
    void renderScreen( fs::path path, int frame ){
        string frame_name = "f_" + toString( frame ) + ".png";
        writeImage( path/frame_name, copyWindowSurface() );
        cout << "Render Image : " << frame << endl;
    }
    
    fs::path getAssetPath(){
        if( assetDir.string()=="" ){
            fs::path p = expandPath("../../../_project_settings.xml");
            XmlTree xml( loadFile(p) );
            XmlTree ast = xml.getChild("project_settings").getChild("assetDir");
            string st = ast.getValue<string>("error");
            if( st == "error"){
                printf( "Cant find assetDir, check you have _project_settings.xml\n" );
            }else{
                assetDir = st;
                printf( "assetDir \"%s\"\n", st.c_str() );
            }
        }
        return assetDir;
    }
    
    fs::path getRenderPath( string subdir_name="" ){
        if(subdir_name!="")
            return expandPath("../../../_rtmp") / getTimeStamp() / subdir_name ;
        else
            return expandPath("../../../_rtmp") / getTimeStamp();
    }
    
    fs::path getProjectPath(){
        return expandPath("../../../");
    }
    
    string getProjectName(){
        
    }
    
    void saveString( string str, fs::path path ){
        ofstream ost( path.string() );
        ost << str;
        ost.close();
    }
    
    float distanceToLine( const Ray &ray, const vec3 &point){
        //return cross(ray.getDirection(), point - ray.getOrigin()).length();
    }
    
    vec3 dirToLine( const vec3 p0, const vec3 &p1, const vec3 &p2 ){
        //vec3 v = p2-p1;
        //vec3 w = p0-p1;
        //double b = v.dot(w) / v.dot(v);
        //return -w + b * v;
    }
    
    void fillSurface( Surface16u & sur, const ColorAf & col){

        Surface16u::Iter itr = sur.getIter();
        while (itr.line() ) {
            while( itr.pixel()){
                //setColorToItr( itr, col );
                sur.setPixel(itr.getPos(), col);
            }
        }
    }
    
    void drawCoordinate( float length=100 ){
        gl::begin( GL_LINES ); {
            gl::color( 1, 0, 0 );
            gl::vertex( 0, 0, 0 );
            gl::vertex( length, 0, 0 );
            gl::color( 0, 1, 0 );
            gl::vertex( 0, 0, 0 );
            gl::vertex( 0, length, 0 );
            gl::color(  0, 0, 1 );
            gl::vertex( 0, 0, 0 );
            gl::vertex( 0, 0, length );
        } gl::end();
    }
  
    void drawScreenGuide(){
        float w = getWindowWidth();
        float h = getWindowHeight();
        gl::pushMatrices();
        gl::setMatricesWindow( getWindowSize() );
        gl::begin( GL_LINES ); {
            gl::color( 1,1,1 );
            gl::vertex( w*0.5, 0, 0 );
            gl::vertex( w*0.5, h, 0 );
            gl::vertex( 0, h*0.5, 0 );
            gl::vertex( w, h*0.5, 0 );
        } gl::end();
        gl::popMatrices();
    }

    void loadColorSample( string fileName, vector<vector<Colorf>>& col){
        Surface sur( loadImage( getAssetPath()/fileName) );
        Surface::Iter itr = sur.getIter();
        
        int w = sur.getWidth();
        int h = sur.getHeight();
        
        col.assign( w, vector<Colorf>(h) );
        
        while ( itr.line() ) {
            
            while ( itr.pixel() ) {
                float r = itr.r() / 255.0f;
                float g = itr.g() / 255.0f;
                float b = itr.b() / 255.0f;
                
                vec2 pos = itr.getPos();
                col[pos.x][pos.y].r = r;
                col[pos.x][pos.y].g = g;
                col[pos.x][pos.y].b = b;
            }
        }
        cout << "ColorSample Load Success: w:" << w << ", h " << h << endl;
    }

    clock_t timer;
    void timer_start(){
        timer = clock();
    }
    
    void timer_end(){
        double elapsedSec = clock() - timer;
        elapsedSec /= CLOCKS_PER_SEC;
        cout << "Elapsed time(sec) : "  <<  elapsedSec << endl;
    }
    
    void setMatricesWindow( int screenWidth, int screenHeight, bool center, bool ydown=true, float near=-10000.0f, float far=10000.0f ){
        auto ctx = gl::context();
        ctx->getModelMatrixStack().back() = mat4();
        ctx->getViewMatrixStack().back() = mat4();
        
        // scale to screen coordinate -1.0 ~ 1.0
        float sx = 2.0f / (float)screenWidth;
        float sy = 2.0f / (float)screenHeight;
        float sz = 2.0f / (far-near);

        // translate
        float tx = -1;
        float ty = -1;
        float tz = -(far+near)/(far-near); // move to mid position, (far+near)/2*sz
        
        if( center ){
            sx *= 0.5;
            sy *= 0.5;
            tx = 0;
            ty = 0;
        }
    
        if( ydown ) {
            sy *= -1;
            ty *= -1;
        }
        
        mat4 &m = ctx->getProjectionMatrixStack().back();
        m = mat4( sx,  0,  0, 0,
                   0, sy,  0, 0,
                   0,  0, sz, 0,
                  tx, ty, tz, 1 );
    }

}