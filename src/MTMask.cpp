//
// Created by Cristobal Mendoza on 2019-07-26.
//

#include <sstream>
#include "MTMask.hpp"
#include "ofGraphics.h"
#include "ofGLUtils.h"

MTMask::MTMask()
{

}

void MTMask::setup(int width, int height)
{
	this->width = width;
	this->height = height;
	buffer.allocate(width, height);
	buffer.begin();
	ofClear(0);
	buffer.end();

	if (ofIsGLProgrammableRenderer())
	{
		std::string vertexShaderSource = generateVertexShader();
		maskShader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShaderSource);
	}

	std::string fragmentShaderSource = generateFragmentShader();
	maskShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShaderSource);

	if (ofIsGLProgrammableRenderer())
	{
		maskShader.bindDefaults();
	}

	maskShader.linkProgram();
}


void MTMask::update(ofTexture& mask, ofTexture& image)
{
	ofPushStyle();
	ofSetColor(255);

	buffer.begin();
	ofClear(0);
	maskShader.begin();
	maskShader.setUniformTexture("maskTex", mask, 1);
	image.draw(0, 0);
	maskShader.end();
	buffer.end();

	ofPopStyle();
}

ofTexture& MTMask::getTexture()
{
	return buffer.getTexture();
}

std::string MTMask::generateFragmentShader()
{
	std::string source;

	if (ofIsGLProgrammableRenderer())
	{
		source = "#version 150\n\
		\n\
		uniform sampler2DRect tex0;\
		uniform sampler2DRect maskTex;\
        in vec2 vTexCoord;\n\
		\
        out vec4 fragColor;\n\
		void main (void){\
		vec2 pos = vTexCoord;\
		\
		vec3 src = texture(tex0, pos).rgb;\
		float mask = texture(maskTex, pos).r;\
		\
		fragColor = vec4( src , mask);\
		}";
	}
	else
	{
		source = "#version 120\n \
		#extension GL_ARB_texture_rectangle : enable\n \
		\
		uniform sampler2DRect tex0;\
		uniform sampler2DRect maskTex;\
		\
		void main (void){\
		vec2 pos = gl_TexCoord[0].st;\
		\
		vec3 src = texture2DRect(tex0, pos).rgb;\
		float mask = texture2DRect(maskTex, pos).r;\
		\
		gl_FragColor = vec4( src , mask);\
		}";
	}

	return source;
}

std::string MTMask::generateVertexShader()
{
	std::stringstream src;

	src <<"#version 150\n";
	src <<"uniform mat4 modelViewProjectionMatrix;\n";
	src <<"in vec4 position;\n";
	src <<"in vec2 texcoord;\n";
	src <<"out vec2 vTexCoord;\n";
	src <<"void main() {;\n";
	src <<"\tvTexCoord = texcoord;\n";
	src <<"\tgl_Position = modelViewProjectionMatrix * position;\n";
	src <<"}\n";

	return src.str();
}
