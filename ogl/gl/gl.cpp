#include "gl.h"

#include <motor/log/global.h>

#if defined( MOTOR_TARGET_OS_WIN )
#include <GL/wglext.h>
#elif defined( MOTOR_TARGET_OS_LIN )
//#include <X11/X.h>
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/Xmd.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

using namespace motor::ogl ;

/////////////////////////////////////////////////////////////////////////
// some defines
/////////////////////////////////////////////////////////////////////////

#define CHECK_AND_LOAD_COND( fn, name ) \
    !motor::log::global_t::error( \
    (fn = (fn == NULL ? (decltype(fn))(this_t::load_gl_function( name )) : fn)) == NULL, \
    "[CHECK_AND_LOAD_COND] : Failed to load: "  name  )

#define CHECK_AND_LOAD( fn, name ) \
{ \
    if( fn == NULL ) \
    { \
        fn = (decltype(fn))(this_t::load_gl_function( name ) ) ; \
    } \
    \
    motor::log::global_t::error( fn == NULL, "[CHECK_AND_LOAD] : Failed to load: "  name  ) ; \
}

#define CHECK_AND_LOAD_BY_FN( fn ) CHECK_AND_LOAD( fn, #fn )

#define JUST_LOAD( fn, name ) \
{ \
    motor::ogl::fn = (decltype(motor::ogl::fn))(this_t::load_gl_function( name ) ) ; \
    motor::log::global_t::error( motor::ogl::fn == NULL, "[JUST_LOAD] : Failed to load: "  name  ) ; \
}
#define JUST_LOAD_BY_FN( fn ) JUST_LOAD( fn, #fn )

#define NULL_STATIC_MEMBER( fn ) decltype(motor::ogl::gl::fn) motor::ogl::gl::fn = nullptr

/////////////////////////////////////////////////////////////////////////
// member init
/////////////////////////////////////////////////////////////////////////

motor::ogl::gl::string_list_t gl::_extensions = motor::ogl::gl::string_list_t( ) ;
bool_t motor::ogl::gl::_init = false ;

#if 0
NULL_STATIC_MEMBER( glCullFace ) ;
NULL_STATIC_MEMBER( glFrontFace ) ;
NULL_STATIC_MEMBER( glHint ) ;
NULL_STATIC_MEMBER( glLineWidth);
NULL_STATIC_MEMBER( glPointSize ) ;
NULL_STATIC_MEMBER( glPolygonMode ) ;
NULL_STATIC_MEMBER( glScissor ) ;
NULL_STATIC_MEMBER( glTexParameterf ) ;
NULL_STATIC_MEMBER( glTexParameterfv ) ;
NULL_STATIC_MEMBER( glTexParameteri ) ;
NULL_STATIC_MEMBER( glTexParameteriv ) ;
NULL_STATIC_MEMBER( glTexImage1D ) ;
NULL_STATIC_MEMBER( glTexImage2D ) ;
NULL_STATIC_MEMBER( glDrawBuffer ) ;
NULL_STATIC_MEMBER( glClear ) ;
NULL_STATIC_MEMBER( glClearColor ) ;
NULL_STATIC_MEMBER( glClearStencil ) ;
NULL_STATIC_MEMBER( glClearDepth ) ;
NULL_STATIC_MEMBER( glStencilMask ) ;
NULL_STATIC_MEMBER( glColorMask ) ;
NULL_STATIC_MEMBER( glDepthMask ) ;
NULL_STATIC_MEMBER( glDisable ) ;
NULL_STATIC_MEMBER( glEnable ) ;
NULL_STATIC_MEMBER( glFinish ) ;
NULL_STATIC_MEMBER( glFlush ) ;
NULL_STATIC_MEMBER( glBlendFunc ) ;
NULL_STATIC_MEMBER( glLogicOp ) ;
NULL_STATIC_MEMBER( glStencilFunc ) ;
NULL_STATIC_MEMBER( glStencilOp ) ;
NULL_STATIC_MEMBER( glDepthFunc ) ;
NULL_STATIC_MEMBER( glPixelStoref ) ;
NULL_STATIC_MEMBER( glPixelStorei ) ;
NULL_STATIC_MEMBER( glReadBuffer ) ;
NULL_STATIC_MEMBER( glReadPixels ) ;
NULL_STATIC_MEMBER( glGetBooleanv ) ;
NULL_STATIC_MEMBER( glGetDoublev ) ;
NULL_STATIC_MEMBER( glGetError ) ;
NULL_STATIC_MEMBER( glGetFloatv ) ;
NULL_STATIC_MEMBER( glGetIntegerv ) ;
NULL_STATIC_MEMBER( glGetString ) ;
NULL_STATIC_MEMBER( glGetTexImage ) ;
NULL_STATIC_MEMBER( glGetTexParameterfv ) ;
NULL_STATIC_MEMBER( glGetTexParameteriv ) ;
NULL_STATIC_MEMBER( glGetTexLevelParameterfv ) ;
NULL_STATIC_MEMBER( glGetTexLevelParameteriv ) ;
NULL_STATIC_MEMBER( glIsEnabled ) ;
NULL_STATIC_MEMBER( glDepthRange ) ;
NULL_STATIC_MEMBER( glViewport ) ;

// gl version 1.1
        
NULL_STATIC_MEMBER( glDrawArrays ) ;
NULL_STATIC_MEMBER( glDrawElements ) ;
NULL_STATIC_MEMBER( glGetPointerv ) ;
NULL_STATIC_MEMBER( glPolygonOffset ) ;
NULL_STATIC_MEMBER( glCopyTexImage1D ) ;
NULL_STATIC_MEMBER( glCopyTexImage2D ) ;
NULL_STATIC_MEMBER( glCopyTexSubImage1D ) ;
NULL_STATIC_MEMBER( glCopyTexSubImage2D ) ;
NULL_STATIC_MEMBER( glTexSubImage1D ) ;
NULL_STATIC_MEMBER( glTexSubImage2D ) ;
NULL_STATIC_MEMBER( glBindTexture ) ;
NULL_STATIC_MEMBER( glDeleteTextures ) ;
NULL_STATIC_MEMBER( glGenTextures ) ;
NULL_STATIC_MEMBER( glIsTexture ) ;


// gl version 1.2

NULL_STATIC_MEMBER( glDrawRangeElements ) ;
NULL_STATIC_MEMBER( glTexImage3D ) ;
NULL_STATIC_MEMBER( glTexSubImage3D ) ;
NULL_STATIC_MEMBER( glCopyTexSubImage3D ) ;

// gl version 1.3

NULL_STATIC_MEMBER( glActiveTexture ) ;
NULL_STATIC_MEMBER( glSampleCoverage ) ;
NULL_STATIC_MEMBER( glCompressedTexImage3D ) ;
NULL_STATIC_MEMBER( glCompressedTexImage2D ) ;
NULL_STATIC_MEMBER( glCompressedTexImage1D ) ;
NULL_STATIC_MEMBER( glCompressedTexSubImage3D ) ;
NULL_STATIC_MEMBER( glCompressedTexSubImage2D ) ;
NULL_STATIC_MEMBER( glCompressedTexSubImage1D ) ;
NULL_STATIC_MEMBER( glGetCompressedTexImage ) ;

// gl version 1.4

NULL_STATIC_MEMBER( glBlendFuncSeparate ) ;
NULL_STATIC_MEMBER( glMultiDrawArrays ) ;
NULL_STATIC_MEMBER( glMultiDrawElements ) ;
NULL_STATIC_MEMBER( glPointParameterf ) ;
NULL_STATIC_MEMBER( glPointParameterfv ) ;
NULL_STATIC_MEMBER( glPointParameteri ) ;
NULL_STATIC_MEMBER( glPointParameteriv ) ;
NULL_STATIC_MEMBER( glBlendColor ) ;
NULL_STATIC_MEMBER( glBlendEquation ) ;

// gl version 1.5

NULL_STATIC_MEMBER( glGenQueries)  ;
NULL_STATIC_MEMBER( glDeleteQueries ) ;
NULL_STATIC_MEMBER( glIsQuery ) ;
NULL_STATIC_MEMBER( glBeginQuery ) ;
NULL_STATIC_MEMBER( glEndQuery ) ;
NULL_STATIC_MEMBER( glGetQueryiv ) ;
NULL_STATIC_MEMBER( glGetQueryObjectiv ) ;
NULL_STATIC_MEMBER( glGetQueryObjectuiv ) ;
NULL_STATIC_MEMBER( glBindBuffer ) ;
NULL_STATIC_MEMBER( glDeleteBuffers ) ;
NULL_STATIC_MEMBER( glGenBuffers ) ;
NULL_STATIC_MEMBER( glIsBuffer ) ;
NULL_STATIC_MEMBER( glBufferData ) ;
NULL_STATIC_MEMBER( glBufferSubData ) ;
NULL_STATIC_MEMBER( glGetBufferSubData ) ;
NULL_STATIC_MEMBER( glMapBuffer ) ;
NULL_STATIC_MEMBER( glUnmapBuffer ) ;
NULL_STATIC_MEMBER( glGetBufferParameteriv ) ;
NULL_STATIC_MEMBER( glGetBufferPointerv ) ;

// gl version 2.0

NULL_STATIC_MEMBER( glBlendEquationSeparate)  ;
NULL_STATIC_MEMBER( glDrawBuffers ) ;
NULL_STATIC_MEMBER( glStencilOpSeparate ) ;
NULL_STATIC_MEMBER( glStencilFuncSeparate ) ;
NULL_STATIC_MEMBER( glStencilMaskSeparate ) ;
NULL_STATIC_MEMBER( glAttachShader ) ;
NULL_STATIC_MEMBER( glBindAttribLocation ) ;
NULL_STATIC_MEMBER( glCompileShader ) ;
NULL_STATIC_MEMBER( glCreateProgram ) ;
NULL_STATIC_MEMBER( glCreateShader ) ;
NULL_STATIC_MEMBER( glDeleteProgram ) ;
NULL_STATIC_MEMBER( glDeleteShader ) ;
NULL_STATIC_MEMBER( glDetachShader ) ;
NULL_STATIC_MEMBER( glDisableVertexAttribArray ) ;
NULL_STATIC_MEMBER( glEnableVertexAttribArray ) ;
NULL_STATIC_MEMBER( glGetActiveAttrib ) ;
NULL_STATIC_MEMBER( glGetActiveUniform ) ;
NULL_STATIC_MEMBER( glGetAttachedShaders ) ; 
NULL_STATIC_MEMBER( glGetAttribLocation ) ;
NULL_STATIC_MEMBER( glGetProgramiv ) ;
NULL_STATIC_MEMBER( glGetProgramInfoLog ) ;
NULL_STATIC_MEMBER( glGetShaderiv ) ;
NULL_STATIC_MEMBER( glGetShaderInfoLog ) ;
NULL_STATIC_MEMBER( glGetShaderSource ) ;
NULL_STATIC_MEMBER( glGetUniformLocation ) ;
NULL_STATIC_MEMBER( glGetUniformfv ) ;
NULL_STATIC_MEMBER( glGetUniformiv ) ;
NULL_STATIC_MEMBER( glGetVertexAttribdv ) ;
NULL_STATIC_MEMBER( glGetVertexAttribfv ) ;
NULL_STATIC_MEMBER( glGetVertexAttribiv ) ;
NULL_STATIC_MEMBER( glGetVertexAttribPointerv ) ;
NULL_STATIC_MEMBER( glIsProgram ) ;
NULL_STATIC_MEMBER( glIsShader ) ;
NULL_STATIC_MEMBER( glLinkProgram ) ;
NULL_STATIC_MEMBER( glShaderSource ) ;
NULL_STATIC_MEMBER( glUseProgram ) ;
NULL_STATIC_MEMBER( glUniform1f ) ;
NULL_STATIC_MEMBER( glUniform2f ) ;
NULL_STATIC_MEMBER( glUniform3f ) ;
NULL_STATIC_MEMBER( glUniform4f ) ;
NULL_STATIC_MEMBER( glUniform1i ) ;
NULL_STATIC_MEMBER( glUniform2i ) ;
NULL_STATIC_MEMBER( glUniform3i ) ;
NULL_STATIC_MEMBER( glUniform4i ) ;
NULL_STATIC_MEMBER( glUniform1fv ) ;
NULL_STATIC_MEMBER( glUniform2fv ) ;
NULL_STATIC_MEMBER( glUniform3fv ) ;
NULL_STATIC_MEMBER( glUniform4fv ) ;
NULL_STATIC_MEMBER( glUniform1iv ) ;
NULL_STATIC_MEMBER( glUniform2iv ) ;
NULL_STATIC_MEMBER( glUniform3iv ) ;
NULL_STATIC_MEMBER( glUniform4iv ) ;
NULL_STATIC_MEMBER( glUniformMatrix2fv ) ;
NULL_STATIC_MEMBER( glUniformMatrix3fv ) ;
NULL_STATIC_MEMBER( glUniformMatrix4fv ) ;
NULL_STATIC_MEMBER( glValidateProgram ) ;
NULL_STATIC_MEMBER( glVertexAttrib1d ) ;
NULL_STATIC_MEMBER( glVertexAttrib1dv ) ;
NULL_STATIC_MEMBER( glVertexAttrib1f ) ;
NULL_STATIC_MEMBER( glVertexAttrib1fv ) ;
NULL_STATIC_MEMBER( glVertexAttrib1s ) ;
NULL_STATIC_MEMBER( glVertexAttrib1sv ) ;
NULL_STATIC_MEMBER( glVertexAttrib2d ) ;
NULL_STATIC_MEMBER( glVertexAttrib2dv ) ;
NULL_STATIC_MEMBER( glVertexAttrib2f ) ;
NULL_STATIC_MEMBER( glVertexAttrib2fv ) ;
NULL_STATIC_MEMBER( glVertexAttrib2s ) ;
NULL_STATIC_MEMBER( glVertexAttrib2sv ) ;
NULL_STATIC_MEMBER( glVertexAttrib3d ) ;
NULL_STATIC_MEMBER( glVertexAttrib3dv ) ;
NULL_STATIC_MEMBER( glVertexAttrib3f ) ;
NULL_STATIC_MEMBER( glVertexAttrib3fv ) ;
NULL_STATIC_MEMBER( glVertexAttrib3s ) ;
NULL_STATIC_MEMBER( glVertexAttrib3sv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4Nbv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4Niv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4Nsv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4Nub ) ;
NULL_STATIC_MEMBER( glVertexAttrib4Nubv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4Nuiv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4Nusv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4bv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4d ) ;
NULL_STATIC_MEMBER( glVertexAttrib4dv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4f ) ;
NULL_STATIC_MEMBER( glVertexAttrib4fv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4iv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4s ) ;
NULL_STATIC_MEMBER( glVertexAttrib4sv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4ubv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4uiv ) ;
NULL_STATIC_MEMBER( glVertexAttrib4usv ) ;
NULL_STATIC_MEMBER( glVertexAttribPointer ) ;

// gl version 2.1

NULL_STATIC_MEMBER( glUniformMatrix2x3fv ) ;
NULL_STATIC_MEMBER( glUniformMatrix3x2fv ) ;
NULL_STATIC_MEMBER( glUniformMatrix2x4fv ) ;
NULL_STATIC_MEMBER( glUniformMatrix4x2fv ) ;
NULL_STATIC_MEMBER( glUniformMatrix3x4fv ) ;
NULL_STATIC_MEMBER( glUniformMatrix4x3fv ) ;

// gl version 3.0

NULL_STATIC_MEMBER( glColorMaski ) ;
NULL_STATIC_MEMBER( glGetBooleani_v ) ;
NULL_STATIC_MEMBER( glGetIntegeri_v ) ;
NULL_STATIC_MEMBER( glEnablei ) ;
NULL_STATIC_MEMBER( glDisablei ) ;
NULL_STATIC_MEMBER( glIsEnabledi ) ;
NULL_STATIC_MEMBER( glBeginTransformFeedback ) ;
NULL_STATIC_MEMBER( glEndTransformFeedback ) ; 
NULL_STATIC_MEMBER( glBindBufferRange ) ;
NULL_STATIC_MEMBER( glBindBufferBase ) ;
NULL_STATIC_MEMBER( glTransformFeedbackVaryings ) ;
NULL_STATIC_MEMBER( glGetTransformFeedbackVarying ) ;
NULL_STATIC_MEMBER( glClampColor ) ;
NULL_STATIC_MEMBER( glBeginConditionalRender ) ;
NULL_STATIC_MEMBER( glEndConditionalRender ) ;
NULL_STATIC_MEMBER( glVertexAttribIPointer ) ;
NULL_STATIC_MEMBER( glGetVertexAttribIiv ) ;
NULL_STATIC_MEMBER( glGetVertexAttribIuiv ) ;
NULL_STATIC_MEMBER( glVertexAttribI1i ) ;
NULL_STATIC_MEMBER( glVertexAttribI2i ) ;
NULL_STATIC_MEMBER( glVertexAttribI3i ) ;
NULL_STATIC_MEMBER( glVertexAttribI4i ) ;
NULL_STATIC_MEMBER( glVertexAttribI1ui ) ;
NULL_STATIC_MEMBER( glVertexAttribI2ui ) ;
NULL_STATIC_MEMBER( glVertexAttribI3ui ) ;
NULL_STATIC_MEMBER( glVertexAttribI4ui ) ;
NULL_STATIC_MEMBER( glVertexAttribI1iv ) ;
NULL_STATIC_MEMBER( glVertexAttribI2iv ) ;
NULL_STATIC_MEMBER( glVertexAttribI3iv ) ;
NULL_STATIC_MEMBER( glVertexAttribI4iv ) ;
NULL_STATIC_MEMBER( glVertexAttribI1uiv ) ;
NULL_STATIC_MEMBER( glVertexAttribI2uiv ) ;
NULL_STATIC_MEMBER( glVertexAttribI3uiv ) ;
NULL_STATIC_MEMBER( glVertexAttribI4uiv ) ;
NULL_STATIC_MEMBER( glVertexAttribI4bv ) ;
NULL_STATIC_MEMBER( glVertexAttribI4sv ) ;
NULL_STATIC_MEMBER( glVertexAttribI4ubv ) ;
NULL_STATIC_MEMBER( glVertexAttribI4usv ) ;
NULL_STATIC_MEMBER( glGetUniformuiv ) ;
NULL_STATIC_MEMBER( glBindFragDataLocation ) ;
NULL_STATIC_MEMBER( glGetFragDataLocation ) ;
NULL_STATIC_MEMBER( glUniform1ui ) ;
NULL_STATIC_MEMBER( glUniform2ui ) ;
NULL_STATIC_MEMBER( glUniform3ui ) ;
NULL_STATIC_MEMBER( glUniform4ui ) ;
NULL_STATIC_MEMBER( glUniform1uiv ) ;
NULL_STATIC_MEMBER( glUniform2uiv ) ;
NULL_STATIC_MEMBER( glUniform3uiv ) ;
NULL_STATIC_MEMBER( glUniform4uiv ) ;
NULL_STATIC_MEMBER( glTexParameterIiv ) ;
NULL_STATIC_MEMBER( glTexParameterIuiv ) ;
NULL_STATIC_MEMBER( glGetTexParameterIiv ) ;
NULL_STATIC_MEMBER( glGetTexParameterIuiv ) ;
NULL_STATIC_MEMBER( glClearBufferiv ) ;
NULL_STATIC_MEMBER( glClearBufferuiv ) ;
NULL_STATIC_MEMBER( glClearBufferfv ) ;
NULL_STATIC_MEMBER( glClearBufferfi ) ;
NULL_STATIC_MEMBER( glGetStringi ) ;
NULL_STATIC_MEMBER( glIsRenderbuffer ) ;
NULL_STATIC_MEMBER( glBindRenderbuffer ) ;
NULL_STATIC_MEMBER( glDeleteRenderbuffers ) ;
NULL_STATIC_MEMBER( glGenRenderbuffers ) ;
NULL_STATIC_MEMBER( glRenderbufferStorage ) ;
NULL_STATIC_MEMBER( glGetRenderbufferParameteriv ) ;
NULL_STATIC_MEMBER( glIsFramebuffer ) ;
NULL_STATIC_MEMBER( glBindFramebuffer ) ;
NULL_STATIC_MEMBER( glDeleteFramebuffers ) ;
NULL_STATIC_MEMBER( glGenFramebuffers ) ;
NULL_STATIC_MEMBER( glCheckFramebufferStatus ) ;
NULL_STATIC_MEMBER( glFramebufferTexture1D ) ;
NULL_STATIC_MEMBER( glFramebufferTexture2D ) ;
NULL_STATIC_MEMBER( glFramebufferTexture3D ) ;
NULL_STATIC_MEMBER( glFramebufferRenderbuffer ) ;
NULL_STATIC_MEMBER( glGetFramebufferAttachmentParameteriv ) ;
NULL_STATIC_MEMBER( glGenerateMipmap ) ;
NULL_STATIC_MEMBER( glBlitFramebuffer ) ;
NULL_STATIC_MEMBER( glRenderbufferStorageMultisample ) ;
NULL_STATIC_MEMBER( glFramebufferTextureLayer ) ;
NULL_STATIC_MEMBER( glMapBufferRange ) ;
NULL_STATIC_MEMBER( glFlushMappedBufferRange ) ;
NULL_STATIC_MEMBER( glBindVertexArray ) ;
NULL_STATIC_MEMBER( glDeleteVertexArrays ) ;
NULL_STATIC_MEMBER( glGenVertexArrays ) ;
NULL_STATIC_MEMBER( glIsVertexArray ) ;
 
// gl version 3.1

NULL_STATIC_MEMBER( glDrawArraysInstanced ) ;
NULL_STATIC_MEMBER( glDrawElementsInstanced ) ;
NULL_STATIC_MEMBER( glTexBuffer ) ;
NULL_STATIC_MEMBER( glPrimitiveRestartIndex ) ;
NULL_STATIC_MEMBER( glCopyBufferSubData ) ;
NULL_STATIC_MEMBER( glGetUniformIndices ) ;
NULL_STATIC_MEMBER( glGetActiveUniformsiv ) ;
NULL_STATIC_MEMBER( glGetActiveUniformName ) ;
NULL_STATIC_MEMBER( glGetUniformBlockIndex ) ;
NULL_STATIC_MEMBER( glGetActiveUniformBlockiv ) ;
NULL_STATIC_MEMBER( glGetActiveUniformBlockName ) ;
NULL_STATIC_MEMBER( glUniformBlockBinding ) ;

// gl version 3.2

NULL_STATIC_MEMBER( glDrawElementsBaseVertex ) ;
NULL_STATIC_MEMBER( glDrawRangeElementsBaseVertex ) ;
NULL_STATIC_MEMBER( glDrawElementsInstancedBaseVertex ) ;
NULL_STATIC_MEMBER( glMultiDrawElementsBaseVertex ) ;
NULL_STATIC_MEMBER( glProvokingVertex ) ;
NULL_STATIC_MEMBER( glFenceSync ) ;
NULL_STATIC_MEMBER( glIsSync ) ;
NULL_STATIC_MEMBER( glDeleteSync ) ;
NULL_STATIC_MEMBER( glClientWaitSync ) ;
NULL_STATIC_MEMBER( glWaitSync ) ;
NULL_STATIC_MEMBER( glGetInteger64v ) ;
NULL_STATIC_MEMBER( glGetSynciv ) ;
NULL_STATIC_MEMBER( glGetInteger64i_v ) ;
NULL_STATIC_MEMBER( glGetBufferParameteri64v ) ;
NULL_STATIC_MEMBER( glFramebufferTexture ) ;
NULL_STATIC_MEMBER( glTexImage2DMultisample ) ;
NULL_STATIC_MEMBER( glTexImage3DMultisample ) ;
NULL_STATIC_MEMBER( glGetMultisamplefv ) ;
NULL_STATIC_MEMBER( glSampleMaski ) ;

// gl version 3.3

NULL_STATIC_MEMBER( glBindFragDataLocationIndexed ) ;
NULL_STATIC_MEMBER( glGetFragDataIndex ) ;
NULL_STATIC_MEMBER( glGenSamplers ) ;
NULL_STATIC_MEMBER( glDeleteSamplers ) ;
NULL_STATIC_MEMBER( glIsSampler ) ;
NULL_STATIC_MEMBER( glBindSampler ) ;
NULL_STATIC_MEMBER( glSamplerParameteri ) ;
NULL_STATIC_MEMBER( glSamplerParameteriv ) ;
NULL_STATIC_MEMBER( glSamplerParameterf ) ;
NULL_STATIC_MEMBER( glSamplerParameterfv ) ;
NULL_STATIC_MEMBER( glSamplerParameterIiv ) ;
NULL_STATIC_MEMBER( glSamplerParameterIuiv ) ;
NULL_STATIC_MEMBER( glGetSamplerParameteriv ) ;
NULL_STATIC_MEMBER( glGetSamplerParameterIiv ) ;
NULL_STATIC_MEMBER( glGetSamplerParameterfv ) ;
NULL_STATIC_MEMBER( glGetSamplerParameterIuiv ) ;
NULL_STATIC_MEMBER( glQueryCounter ) ;
NULL_STATIC_MEMBER( glGetQueryObjecti64v ) ;
NULL_STATIC_MEMBER( glGetQueryObjectui64v ) ;
NULL_STATIC_MEMBER( glVertexAttribDivisor ) ;
NULL_STATIC_MEMBER( glVertexAttribP1ui ) ;
NULL_STATIC_MEMBER( glVertexAttribP1uiv ) ;
NULL_STATIC_MEMBER( glVertexAttribP2ui ) ;
NULL_STATIC_MEMBER( glVertexAttribP2uiv ) ;
NULL_STATIC_MEMBER( glVertexAttribP3ui ) ;
NULL_STATIC_MEMBER( glVertexAttribP3uiv ) ;
NULL_STATIC_MEMBER( glVertexAttribP4ui ) ;
NULL_STATIC_MEMBER( glVertexAttribP4uiv ) ;

// gl version 4.0

NULL_STATIC_MEMBER( glMinSampleShading ) ;
NULL_STATIC_MEMBER( glBlendEquationi ) ;
NULL_STATIC_MEMBER( glBlendEquationSeparatei ) ;
NULL_STATIC_MEMBER( glBlendFunci ) ;
NULL_STATIC_MEMBER( glBlendFuncSeparatei ) ;
NULL_STATIC_MEMBER( glDrawArraysIndirect ) ;
NULL_STATIC_MEMBER( glDrawElementsIndirect ) ;
NULL_STATIC_MEMBER( glUniform1d ) ;
NULL_STATIC_MEMBER( glUniform2d ) ;
NULL_STATIC_MEMBER( glUniform3d ) ;
NULL_STATIC_MEMBER( glUniform4d ) ;
NULL_STATIC_MEMBER( glUniform1dv ) ;
NULL_STATIC_MEMBER( glUniform2dv ) ;
NULL_STATIC_MEMBER( glUniform3dv ) ;
NULL_STATIC_MEMBER( glUniform4dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix2dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix3dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix4dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix2x3dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix2x4dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix3x2dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix3x4dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix4x2dv ) ;
NULL_STATIC_MEMBER( glUniformMatrix4x3dv ) ;
NULL_STATIC_MEMBER( glGetUniformdv ) ;
NULL_STATIC_MEMBER( glGetSubroutineUniformLocation ) ;
NULL_STATIC_MEMBER( glGetSubroutineIndex ) ;
NULL_STATIC_MEMBER( glGetActiveSubroutineUniformiv ) ;
NULL_STATIC_MEMBER( glGetActiveSubroutineUniformName ) ;
NULL_STATIC_MEMBER( glGetActiveSubroutineName ) ;
NULL_STATIC_MEMBER( glUniformSubroutinesuiv ) ;
NULL_STATIC_MEMBER( glGetUniformSubroutineuiv ) ;
NULL_STATIC_MEMBER( glGetProgramStageiv ) ;
NULL_STATIC_MEMBER( glPatchParameteri ) ;
NULL_STATIC_MEMBER( glPatchParameterfv ) ;
NULL_STATIC_MEMBER( glBindTransformFeedback ) ;
NULL_STATIC_MEMBER( glDeleteTransformFeedbacks ) ;
NULL_STATIC_MEMBER( glGenTransformFeedbacks ) ;
NULL_STATIC_MEMBER( glIsTransformFeedback ) ;
NULL_STATIC_MEMBER( glPauseTransformFeedback ) ;
NULL_STATIC_MEMBER( glResumeTransformFeedback ) ;
NULL_STATIC_MEMBER( glDrawTransformFeedback ) ;
NULL_STATIC_MEMBER( glDrawTransformFeedbackStream ) ;
NULL_STATIC_MEMBER( glBeginQueryIndexed ) ;
NULL_STATIC_MEMBER( glEndQueryIndexed ) ;
NULL_STATIC_MEMBER( glGetQueryIndexediv ) ;
 
// gl version 4.1
         
NULL_STATIC_MEMBER( glReleaseShaderCompiler ) ;
NULL_STATIC_MEMBER( glShaderBinary ) ;
NULL_STATIC_MEMBER( glGetShaderPrecisionFormat ) ;
NULL_STATIC_MEMBER( glDepthRangef ) ;
NULL_STATIC_MEMBER( glClearDepthf ) ;
NULL_STATIC_MEMBER( glGetProgramBinary ) ;
NULL_STATIC_MEMBER( glProgramBinary ) ;
NULL_STATIC_MEMBER( glProgramParameteri ) ;
NULL_STATIC_MEMBER( glUseProgramStages ) ;
NULL_STATIC_MEMBER( glActiveShaderProgram ) ;
NULL_STATIC_MEMBER( glCreateShaderProgramv ) ;
NULL_STATIC_MEMBER( glBindProgramPipeline ) ;
NULL_STATIC_MEMBER( glDeleteProgramPipelines ) ;
NULL_STATIC_MEMBER( glGenProgramPipelines ) ;
NULL_STATIC_MEMBER( glIsProgramPipeline ) ;
NULL_STATIC_MEMBER( glGetProgramPipelineiv ) ;
NULL_STATIC_MEMBER( glProgramUniform1i ) ;
NULL_STATIC_MEMBER( glProgramUniform1iv ) ;
NULL_STATIC_MEMBER( glProgramUniform1f ) ;
NULL_STATIC_MEMBER( glProgramUniform1fv ) ;
NULL_STATIC_MEMBER( glProgramUniform1d ) ;
NULL_STATIC_MEMBER( glProgramUniform1dv ) ;
NULL_STATIC_MEMBER( glProgramUniform1ui ) ;
NULL_STATIC_MEMBER( glProgramUniform1uiv ) ;
NULL_STATIC_MEMBER( glProgramUniform2i ) ;
NULL_STATIC_MEMBER( glProgramUniform2iv ) ;
NULL_STATIC_MEMBER( glProgramUniform2f ) ;
NULL_STATIC_MEMBER( glProgramUniform2fv ) ;
NULL_STATIC_MEMBER( glProgramUniform2d ) ;
NULL_STATIC_MEMBER( glProgramUniform2dv ) ;
NULL_STATIC_MEMBER( glProgramUniform2ui ) ;
NULL_STATIC_MEMBER( glProgramUniform2uiv ) ;
NULL_STATIC_MEMBER( glProgramUniform3i ) ;
NULL_STATIC_MEMBER( glProgramUniform3iv ) ;
NULL_STATIC_MEMBER( glProgramUniform3f ) ;
NULL_STATIC_MEMBER( glProgramUniform3fv ) ;
NULL_STATIC_MEMBER( glProgramUniform3d ) ;
NULL_STATIC_MEMBER( glProgramUniform3dv ) ;
NULL_STATIC_MEMBER( glProgramUniform3ui ) ;
NULL_STATIC_MEMBER( glProgramUniform3uiv ) ;
NULL_STATIC_MEMBER( glProgramUniform4i ) ;
NULL_STATIC_MEMBER( glProgramUniform4iv ) ;
NULL_STATIC_MEMBER( glProgramUniform4f ) ;
NULL_STATIC_MEMBER( glProgramUniform4fv ) ;
NULL_STATIC_MEMBER( glProgramUniform4d ) ;
NULL_STATIC_MEMBER( glProgramUniform4dv ) ;
NULL_STATIC_MEMBER( glProgramUniform4ui ) ;
NULL_STATIC_MEMBER( glProgramUniform4uiv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix2fv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix3fv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix4fv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix2dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix3dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix4dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix2x3fv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix3x2fv ) ; 
NULL_STATIC_MEMBER( glProgramUniformMatrix2x4fv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix4x2fv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix3x4fv ) ; 
NULL_STATIC_MEMBER( glProgramUniformMatrix4x3fv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix2x3dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix3x2dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix2x4dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix4x2dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix3x4dv ) ;
NULL_STATIC_MEMBER( glProgramUniformMatrix4x3dv ) ;
NULL_STATIC_MEMBER( glValidateProgramPipeline ) ;
NULL_STATIC_MEMBER( glGetProgramPipelineInfoLog ) ; 
NULL_STATIC_MEMBER( glVertexAttribL1d  ) ;
NULL_STATIC_MEMBER( glVertexAttribL2d ) ;
NULL_STATIC_MEMBER( glVertexAttribL3d ) ;
NULL_STATIC_MEMBER( glVertexAttribL4d ) ;
NULL_STATIC_MEMBER( glVertexAttribL1dv ) ;
NULL_STATIC_MEMBER( glVertexAttribL2dv ) ;
NULL_STATIC_MEMBER( glVertexAttribL3dv ) ;
NULL_STATIC_MEMBER( glVertexAttribL4dv ) ;
NULL_STATIC_MEMBER( glVertexAttribLPointer ) ;
NULL_STATIC_MEMBER( glGetVertexAttribLdv ) ;
NULL_STATIC_MEMBER( glViewportArrayv ) ;
NULL_STATIC_MEMBER( glViewportIndexedf ) ;
NULL_STATIC_MEMBER( glViewportIndexedfv ) ;
NULL_STATIC_MEMBER( glScissorArrayv ) ;
NULL_STATIC_MEMBER( glScissorIndexed ) ;
NULL_STATIC_MEMBER( glScissorIndexedv ) ;
NULL_STATIC_MEMBER( glDepthRangeArrayv ) ;
NULL_STATIC_MEMBER( glDepthRangeIndexed ) ;
NULL_STATIC_MEMBER( glGetFloati_v ) ;
NULL_STATIC_MEMBER( glGetDoublei_v ) ;
#endif
/////////////////////////////////////////////////////////////////////////
// Function definitions
/////////////////////////////////////////////////////////////////////////

//**************************************************************
void_ptr_t motor::ogl::gl::load_gl_function( char_cptr_t name ) 
{
#if defined( MOTOR_TARGET_OS_WIN )
    void *p = (void *)wglGetProcAddress(name);
    if (p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1))
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);
    }
#elif defined( MOTOR_TARGET_OS_LIN )
    void_ptr_t p = (void_ptr_t) glXGetProcAddress( (GLubyte const *) name ) ;
#else
#error "Requires implementation"
#endif
    return p;
}

//**************************************************************
bool_t motor::ogl::gl::is_supported( char const * name ) 
{
    auto const iter = std::find( _extensions.begin(), _extensions.end(), name ) ;
    return iter != _extensions.end() ;
}

//**************************************************************
motor::ogl::result motor::ogl::gl::init( void_t ) 
{
    //if( _init ) return motor::ogl::result::ok ;

    if( !CHECK_AND_LOAD_COND( glGetIntegerv, "glGetIntegerv" ) )
    {
        return motor::ogl::result::failed_load_function ;
    }

    if( !CHECK_AND_LOAD_COND( glGetStringi, "glGetStringi" ) )
    {

    }

    // load all supported extension strings
    {
        GLint numext = 0 ;
        motor::ogl::glGetIntegerv( GL_NUM_EXTENSIONS, &numext  ) ;

        for( GLint i=0; i<numext; ++i )
        {
            GLubyte const * name = motor::ogl::glGetStringi( GL_EXTENSIONS, i  ) ;
            _extensions.push_back( reinterpret_cast<char const*>( name )) ;
        }
    }
    
    JUST_LOAD_BY_FN( glCullFace ) 
    JUST_LOAD_BY_FN( glFrontFace ) 
    JUST_LOAD_BY_FN( glHint ) 
    JUST_LOAD_BY_FN( glLineWidth) 
    JUST_LOAD_BY_FN( glPointSize ) 
    JUST_LOAD_BY_FN( glPolygonMode ) 
    JUST_LOAD_BY_FN( glScissor ) 
    JUST_LOAD_BY_FN( glTexParameterf ) 
    JUST_LOAD_BY_FN( glTexParameterfv ) 
    JUST_LOAD_BY_FN( glTexParameteri ) 
    JUST_LOAD_BY_FN( glTexParameteriv ) 
    JUST_LOAD_BY_FN( glTexImage1D ) 
    JUST_LOAD_BY_FN( glTexImage2D ) 
    JUST_LOAD_BY_FN( glDrawBuffer ) 
    JUST_LOAD_BY_FN( glClear ) 
    JUST_LOAD_BY_FN( glClearColor ) 
    JUST_LOAD_BY_FN( glClearStencil ) 
    JUST_LOAD_BY_FN( glClearDepth ) 
    JUST_LOAD_BY_FN( glStencilMask ) 
    JUST_LOAD_BY_FN( glColorMask ) 
    JUST_LOAD_BY_FN( glDepthMask ) 
    JUST_LOAD_BY_FN( glDisable ) 
    JUST_LOAD_BY_FN( glEnable ) 
    JUST_LOAD_BY_FN( glFinish ) 
    JUST_LOAD_BY_FN( glFlush ) 
    JUST_LOAD_BY_FN( glBlendFunc ) 
    JUST_LOAD_BY_FN( glLogicOp ) 
    JUST_LOAD_BY_FN( glStencilFunc ) 
    JUST_LOAD_BY_FN( glStencilOp ) 
    JUST_LOAD_BY_FN( glDepthFunc ) 
    JUST_LOAD_BY_FN( glPixelStoref ) 
    JUST_LOAD_BY_FN( glPixelStorei ) 
    JUST_LOAD_BY_FN( glReadBuffer ) 
    JUST_LOAD_BY_FN( glReadPixels ) 
    JUST_LOAD_BY_FN( glGetBooleanv ) 
    JUST_LOAD_BY_FN( glGetDoublev ) 
    JUST_LOAD_BY_FN( glGetError ) 
    JUST_LOAD_BY_FN( glGetFloatv ) 
    JUST_LOAD_BY_FN( glGetIntegerv ) 
    JUST_LOAD_BY_FN( glGetString ) 
    JUST_LOAD_BY_FN( glGetTexImage ) 
    JUST_LOAD_BY_FN( glGetTexParameterfv ) 
    JUST_LOAD_BY_FN( glGetTexParameteriv ) 
    JUST_LOAD_BY_FN( glGetTexLevelParameterfv ) 
    JUST_LOAD_BY_FN( glGetTexLevelParameteriv ) 
    JUST_LOAD_BY_FN( glIsEnabled ) 
    JUST_LOAD_BY_FN( glDepthRange ) 
    JUST_LOAD_BY_FN( glViewport ) 

    // gl version 1.1
        
    JUST_LOAD_BY_FN( glDrawArrays ) 
    JUST_LOAD_BY_FN( glDrawElements ) 
    JUST_LOAD_BY_FN( glGetPointerv ) 
    JUST_LOAD_BY_FN( glPolygonOffset ) 
    JUST_LOAD_BY_FN( glCopyTexImage1D ) 
    JUST_LOAD_BY_FN( glCopyTexImage2D ) 
    JUST_LOAD_BY_FN( glCopyTexSubImage1D ) 
    JUST_LOAD_BY_FN( glCopyTexSubImage2D ) 
    JUST_LOAD_BY_FN( glTexSubImage1D ) 
    JUST_LOAD_BY_FN( glTexSubImage2D ) 
    JUST_LOAD_BY_FN( glBindTexture ) 
    JUST_LOAD_BY_FN( glDeleteTextures ) 
    JUST_LOAD_BY_FN( glGenTextures ) 
    JUST_LOAD_BY_FN( glIsTexture ) 


    // gl version 1.2

    JUST_LOAD_BY_FN( glDrawRangeElements ) 
    JUST_LOAD_BY_FN( glTexImage3D ) 
    JUST_LOAD_BY_FN( glTexSubImage3D ) 
    JUST_LOAD_BY_FN( glCopyTexSubImage3D ) 

    // gl version 1.3

    JUST_LOAD_BY_FN( glActiveTexture ) 
    JUST_LOAD_BY_FN( glSampleCoverage ) 
    JUST_LOAD_BY_FN( glCompressedTexImage3D ) 
    JUST_LOAD_BY_FN( glCompressedTexImage2D ) 
    JUST_LOAD_BY_FN( glCompressedTexImage1D ) 
    JUST_LOAD_BY_FN( glCompressedTexSubImage3D ) 
    JUST_LOAD_BY_FN( glCompressedTexSubImage2D ) 
    JUST_LOAD_BY_FN( glCompressedTexSubImage1D ) 
    JUST_LOAD_BY_FN( glGetCompressedTexImage ) 

    // gl version 1.4

    JUST_LOAD_BY_FN( glBlendFuncSeparate ) 
    JUST_LOAD_BY_FN( glMultiDrawArrays ) 
    JUST_LOAD_BY_FN( glMultiDrawElements ) 
    JUST_LOAD_BY_FN( glPointParameterf ) 
    JUST_LOAD_BY_FN( glPointParameterfv ) 
    JUST_LOAD_BY_FN( glPointParameteri ) 
    JUST_LOAD_BY_FN( glPointParameteriv ) 
    JUST_LOAD_BY_FN( glBlendColor ) 
    JUST_LOAD_BY_FN( glBlendEquation ) 

    // gl version 1.5

    JUST_LOAD_BY_FN( glGenQueries)  
    JUST_LOAD_BY_FN( glDeleteQueries ) 
    JUST_LOAD_BY_FN( glIsQuery ) 
    JUST_LOAD_BY_FN( glBeginQuery ) 
    JUST_LOAD_BY_FN( glEndQuery ) 
    JUST_LOAD_BY_FN( glGetQueryiv ) 
    JUST_LOAD_BY_FN( glGetQueryObjectiv ) 
    JUST_LOAD_BY_FN( glGetQueryObjectuiv ) 
    JUST_LOAD_BY_FN( glBindBuffer ) 
    JUST_LOAD_BY_FN( glDeleteBuffers ) 
    JUST_LOAD_BY_FN( glGenBuffers ) 
    JUST_LOAD_BY_FN( glIsBuffer ) 
    JUST_LOAD_BY_FN( glBufferData ) 
    JUST_LOAD_BY_FN( glBufferSubData ) 
    JUST_LOAD_BY_FN( glGetBufferSubData ) 
    JUST_LOAD_BY_FN( glMapBuffer ) 
    JUST_LOAD_BY_FN( glUnmapBuffer ) 
    JUST_LOAD_BY_FN( glGetBufferParameteriv ) 
    JUST_LOAD_BY_FN( glGetBufferPointerv ) 

    // gl version 2.0

    JUST_LOAD_BY_FN( glBlendEquationSeparate)  
    JUST_LOAD_BY_FN( glDrawBuffers ) 
    JUST_LOAD_BY_FN( glStencilOpSeparate ) 
    JUST_LOAD_BY_FN( glStencilFuncSeparate ) 
    JUST_LOAD_BY_FN( glStencilMaskSeparate ) 
    JUST_LOAD_BY_FN( glAttachShader ) 
    JUST_LOAD_BY_FN( glBindAttribLocation ) 
    JUST_LOAD_BY_FN( glCompileShader ) 
    JUST_LOAD_BY_FN( glCreateProgram ) 
    JUST_LOAD_BY_FN( glCreateShader ) 
    JUST_LOAD_BY_FN( glDeleteProgram ) 
    JUST_LOAD_BY_FN( glDeleteShader ) 
    JUST_LOAD_BY_FN( glDetachShader ) 
    JUST_LOAD_BY_FN( glDisableVertexAttribArray ) 
    JUST_LOAD_BY_FN( glEnableVertexAttribArray ) 
    JUST_LOAD_BY_FN( glGetActiveAttrib ) 
    JUST_LOAD_BY_FN( glGetActiveUniform ) 
    JUST_LOAD_BY_FN( glGetAttachedShaders )  
    JUST_LOAD_BY_FN( glGetAttribLocation ) 
    JUST_LOAD_BY_FN( glGetProgramiv ) 
    JUST_LOAD_BY_FN( glGetProgramInfoLog ) 
    JUST_LOAD_BY_FN( glGetShaderiv ) 
    JUST_LOAD_BY_FN( glGetShaderInfoLog ) 
    JUST_LOAD_BY_FN( glGetShaderSource ) 
    JUST_LOAD_BY_FN( glGetUniformLocation ) 
    JUST_LOAD_BY_FN( glGetUniformfv ) 
    JUST_LOAD_BY_FN( glGetUniformiv ) 
    JUST_LOAD_BY_FN( glGetVertexAttribdv ) 
    JUST_LOAD_BY_FN( glGetVertexAttribfv ) 
    JUST_LOAD_BY_FN( glGetVertexAttribiv ) 
    JUST_LOAD_BY_FN( glGetVertexAttribPointerv ) 
    JUST_LOAD_BY_FN( glIsProgram ) 
    JUST_LOAD_BY_FN( glIsShader ) 
    JUST_LOAD_BY_FN( glLinkProgram ) 
    JUST_LOAD_BY_FN( glShaderSource ) 
    JUST_LOAD_BY_FN( glUseProgram ) 
    JUST_LOAD_BY_FN( glUniform1f ) 
    JUST_LOAD_BY_FN( glUniform2f ) 
    JUST_LOAD_BY_FN( glUniform3f ) 
    JUST_LOAD_BY_FN( glUniform4f ) 
    JUST_LOAD_BY_FN( glUniform1i ) 
    JUST_LOAD_BY_FN( glUniform2i ) 
    JUST_LOAD_BY_FN( glUniform3i ) 
    JUST_LOAD_BY_FN( glUniform4i ) 
    JUST_LOAD_BY_FN( glUniform1fv ) 
    JUST_LOAD_BY_FN( glUniform2fv ) 
    JUST_LOAD_BY_FN( glUniform3fv ) 
    JUST_LOAD_BY_FN( glUniform4fv ) 
    JUST_LOAD_BY_FN( glUniform1iv ) 
    JUST_LOAD_BY_FN( glUniform2iv ) 
    JUST_LOAD_BY_FN( glUniform3iv ) 
    JUST_LOAD_BY_FN( glUniform4iv ) 
    JUST_LOAD_BY_FN( glUniformMatrix2fv ) 
    JUST_LOAD_BY_FN( glUniformMatrix3fv ) 
    JUST_LOAD_BY_FN( glUniformMatrix4fv ) 
    JUST_LOAD_BY_FN( glValidateProgram ) 
    JUST_LOAD_BY_FN( glVertexAttrib1d ) 
    JUST_LOAD_BY_FN( glVertexAttrib1dv ) 
    JUST_LOAD_BY_FN( glVertexAttrib1f ) 
    JUST_LOAD_BY_FN( glVertexAttrib1fv ) 
    JUST_LOAD_BY_FN( glVertexAttrib1s ) 
    JUST_LOAD_BY_FN( glVertexAttrib1sv ) 
    JUST_LOAD_BY_FN( glVertexAttrib2d ) 
    JUST_LOAD_BY_FN( glVertexAttrib2dv ) 
    JUST_LOAD_BY_FN( glVertexAttrib2f ) 
    JUST_LOAD_BY_FN( glVertexAttrib2fv ) 
    JUST_LOAD_BY_FN( glVertexAttrib2s ) 
    JUST_LOAD_BY_FN( glVertexAttrib2sv ) 
    JUST_LOAD_BY_FN( glVertexAttrib3d ) 
    JUST_LOAD_BY_FN( glVertexAttrib3dv ) 
    JUST_LOAD_BY_FN( glVertexAttrib3f ) 
    JUST_LOAD_BY_FN( glVertexAttrib3fv ) 
    JUST_LOAD_BY_FN( glVertexAttrib3s ) 
    JUST_LOAD_BY_FN( glVertexAttrib3sv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4Nbv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4Niv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4Nsv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4Nub ) 
    JUST_LOAD_BY_FN( glVertexAttrib4Nubv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4Nuiv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4Nusv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4bv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4d ) 
    JUST_LOAD_BY_FN( glVertexAttrib4dv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4f ) 
    JUST_LOAD_BY_FN( glVertexAttrib4fv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4iv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4s ) 
    JUST_LOAD_BY_FN( glVertexAttrib4sv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4ubv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4uiv ) 
    JUST_LOAD_BY_FN( glVertexAttrib4usv ) 
    JUST_LOAD_BY_FN( glVertexAttribPointer ) 

    // gl version 2.1

    JUST_LOAD_BY_FN( glUniformMatrix2x3fv ) 
    JUST_LOAD_BY_FN( glUniformMatrix3x2fv ) 
    JUST_LOAD_BY_FN( glUniformMatrix2x4fv ) 
    JUST_LOAD_BY_FN( glUniformMatrix4x2fv ) 
    JUST_LOAD_BY_FN( glUniformMatrix3x4fv ) 
    JUST_LOAD_BY_FN( glUniformMatrix4x3fv ) 

    // gl version 3.0

    JUST_LOAD_BY_FN( glColorMaski ) 
    JUST_LOAD_BY_FN( glGetBooleani_v ) 
    JUST_LOAD_BY_FN( glGetIntegeri_v ) 
    JUST_LOAD_BY_FN( glEnablei ) 
    JUST_LOAD_BY_FN( glDisablei ) 
    JUST_LOAD_BY_FN( glIsEnabledi ) 
    JUST_LOAD_BY_FN( glBeginTransformFeedback ) 
    JUST_LOAD_BY_FN( glEndTransformFeedback )  
    JUST_LOAD_BY_FN( glBindBufferRange ) 
    JUST_LOAD_BY_FN( glBindBufferBase ) 
    JUST_LOAD_BY_FN( glTransformFeedbackVaryings ) 
    JUST_LOAD_BY_FN( glGetTransformFeedbackVarying ) 
    JUST_LOAD_BY_FN( glClampColor ) 
    JUST_LOAD_BY_FN( glBeginConditionalRender ) 
    JUST_LOAD_BY_FN( glEndConditionalRender ) 
    JUST_LOAD_BY_FN( glVertexAttribIPointer ) 
    JUST_LOAD_BY_FN( glGetVertexAttribIiv ) 
    JUST_LOAD_BY_FN( glGetVertexAttribIuiv ) 
    JUST_LOAD_BY_FN( glVertexAttribI1i ) 
    JUST_LOAD_BY_FN( glVertexAttribI2i ) 
    JUST_LOAD_BY_FN( glVertexAttribI3i ) 
    JUST_LOAD_BY_FN( glVertexAttribI4i ) 
    JUST_LOAD_BY_FN( glVertexAttribI1ui ) 
    JUST_LOAD_BY_FN( glVertexAttribI2ui ) 
    JUST_LOAD_BY_FN( glVertexAttribI3ui ) 
    JUST_LOAD_BY_FN( glVertexAttribI4ui ) 
    JUST_LOAD_BY_FN( glVertexAttribI1iv ) 
    JUST_LOAD_BY_FN( glVertexAttribI2iv ) 
    JUST_LOAD_BY_FN( glVertexAttribI3iv ) 
    JUST_LOAD_BY_FN( glVertexAttribI4iv ) 
    JUST_LOAD_BY_FN( glVertexAttribI1uiv ) 
    JUST_LOAD_BY_FN( glVertexAttribI2uiv ) 
    JUST_LOAD_BY_FN( glVertexAttribI3uiv ) 
    JUST_LOAD_BY_FN( glVertexAttribI4uiv ) 
    JUST_LOAD_BY_FN( glVertexAttribI4bv ) 
    JUST_LOAD_BY_FN( glVertexAttribI4sv ) 
    JUST_LOAD_BY_FN( glVertexAttribI4ubv ) 
    JUST_LOAD_BY_FN( glVertexAttribI4usv ) 
    JUST_LOAD_BY_FN( glGetUniformuiv ) 
    JUST_LOAD_BY_FN( glBindFragDataLocation ) 
    JUST_LOAD_BY_FN( glGetFragDataLocation ) 
    JUST_LOAD_BY_FN( glUniform1ui ) 
    JUST_LOAD_BY_FN( glUniform2ui ) 
    JUST_LOAD_BY_FN( glUniform3ui ) 
    JUST_LOAD_BY_FN( glUniform4ui ) 
    JUST_LOAD_BY_FN( glUniform1uiv ) 
    JUST_LOAD_BY_FN( glUniform2uiv ) 
    JUST_LOAD_BY_FN( glUniform3uiv ) 
    JUST_LOAD_BY_FN( glUniform4uiv ) 
    JUST_LOAD_BY_FN( glTexParameterIiv ) 
    JUST_LOAD_BY_FN( glTexParameterIuiv ) 
    JUST_LOAD_BY_FN( glGetTexParameterIiv ) 
    JUST_LOAD_BY_FN( glGetTexParameterIuiv ) 
    JUST_LOAD_BY_FN( glClearBufferiv ) 
    JUST_LOAD_BY_FN( glClearBufferuiv ) 
    JUST_LOAD_BY_FN( glClearBufferfv ) 
    JUST_LOAD_BY_FN( glClearBufferfi ) 
    JUST_LOAD_BY_FN( glGetStringi ) 
    JUST_LOAD_BY_FN( glIsRenderbuffer ) 
    JUST_LOAD_BY_FN( glBindRenderbuffer ) 
    JUST_LOAD_BY_FN( glDeleteRenderbuffers ) 
    JUST_LOAD_BY_FN( glGenRenderbuffers ) 
    JUST_LOAD_BY_FN( glRenderbufferStorage ) 
    JUST_LOAD_BY_FN( glGetRenderbufferParameteriv ) 
    JUST_LOAD_BY_FN( glIsFramebuffer ) 
    JUST_LOAD_BY_FN( glBindFramebuffer ) 
    JUST_LOAD_BY_FN( glDeleteFramebuffers ) 
    JUST_LOAD_BY_FN( glGenFramebuffers ) 
    JUST_LOAD_BY_FN( glCheckFramebufferStatus ) 
    JUST_LOAD_BY_FN( glFramebufferTexture1D ) 
    JUST_LOAD_BY_FN( glFramebufferTexture2D ) 
    JUST_LOAD_BY_FN( glFramebufferTexture3D ) 
    JUST_LOAD_BY_FN( glFramebufferRenderbuffer ) 
    JUST_LOAD_BY_FN( glGetFramebufferAttachmentParameteriv ) 
    JUST_LOAD_BY_FN( glGenerateMipmap ) 
    JUST_LOAD_BY_FN( glBlitFramebuffer ) 
    JUST_LOAD_BY_FN( glRenderbufferStorageMultisample ) 
    JUST_LOAD_BY_FN( glFramebufferTextureLayer ) 
    JUST_LOAD_BY_FN( glMapBufferRange ) 
    JUST_LOAD_BY_FN( glFlushMappedBufferRange ) 
    JUST_LOAD_BY_FN( glBindVertexArray ) 
    JUST_LOAD_BY_FN( glDeleteVertexArrays ) 
    JUST_LOAD_BY_FN( glGenVertexArrays ) 
    JUST_LOAD_BY_FN( glIsVertexArray ) 
 
    // gl version 3.1

    JUST_LOAD_BY_FN( glDrawArraysInstanced ) 
    JUST_LOAD_BY_FN( glDrawElementsInstanced ) 
    JUST_LOAD_BY_FN( glTexBuffer ) 
    JUST_LOAD_BY_FN( glPrimitiveRestartIndex ) 
    JUST_LOAD_BY_FN( glCopyBufferSubData ) 
    JUST_LOAD_BY_FN( glGetUniformIndices ) 
    JUST_LOAD_BY_FN( glGetActiveUniformsiv ) 
    JUST_LOAD_BY_FN( glGetActiveUniformName ) 
    JUST_LOAD_BY_FN( glGetUniformBlockIndex ) 
    JUST_LOAD_BY_FN( glGetActiveUniformBlockiv ) 
    JUST_LOAD_BY_FN( glGetActiveUniformBlockName ) 
    JUST_LOAD_BY_FN( glUniformBlockBinding ) 

    // gl version 3.2

    JUST_LOAD_BY_FN( glDrawElementsBaseVertex ) 
    JUST_LOAD_BY_FN( glDrawRangeElementsBaseVertex ) 
    JUST_LOAD_BY_FN( glDrawElementsInstancedBaseVertex ) 
    JUST_LOAD_BY_FN( glMultiDrawElementsBaseVertex ) 
    JUST_LOAD_BY_FN( glProvokingVertex ) 
    JUST_LOAD_BY_FN( glFenceSync ) 
    JUST_LOAD_BY_FN( glIsSync ) 
    JUST_LOAD_BY_FN( glDeleteSync ) 
    JUST_LOAD_BY_FN( glClientWaitSync ) 
    JUST_LOAD_BY_FN( glWaitSync ) 
    JUST_LOAD_BY_FN( glGetInteger64v ) 
    JUST_LOAD_BY_FN( glGetSynciv ) 
    JUST_LOAD_BY_FN( glGetInteger64i_v ) 
    JUST_LOAD_BY_FN( glGetBufferParameteri64v ) 
    JUST_LOAD_BY_FN( glFramebufferTexture ) 
    JUST_LOAD_BY_FN( glTexImage2DMultisample ) 
    JUST_LOAD_BY_FN( glTexImage3DMultisample ) 
    JUST_LOAD_BY_FN( glGetMultisamplefv ) 
    JUST_LOAD_BY_FN( glSampleMaski ) 

    // gl version 3.3

    JUST_LOAD_BY_FN( glBindFragDataLocationIndexed ) 
    JUST_LOAD_BY_FN( glGetFragDataIndex ) 
    JUST_LOAD_BY_FN( glGenSamplers ) 
    JUST_LOAD_BY_FN( glDeleteSamplers ) 
    JUST_LOAD_BY_FN( glIsSampler ) 
    JUST_LOAD_BY_FN( glBindSampler ) 
    JUST_LOAD_BY_FN( glSamplerParameteri ) 
    JUST_LOAD_BY_FN( glSamplerParameteriv ) 
    JUST_LOAD_BY_FN( glSamplerParameterf ) 
    JUST_LOAD_BY_FN( glSamplerParameterfv ) 
    JUST_LOAD_BY_FN( glSamplerParameterIiv ) 
    JUST_LOAD_BY_FN( glSamplerParameterIuiv ) 
    JUST_LOAD_BY_FN( glGetSamplerParameteriv ) 
    JUST_LOAD_BY_FN( glGetSamplerParameterIiv ) 
    JUST_LOAD_BY_FN( glGetSamplerParameterfv ) 
    JUST_LOAD_BY_FN( glGetSamplerParameterIuiv ) 
    JUST_LOAD_BY_FN( glQueryCounter ) 
    JUST_LOAD_BY_FN( glGetQueryObjecti64v ) 
    JUST_LOAD_BY_FN( glGetQueryObjectui64v ) 
    JUST_LOAD_BY_FN( glVertexAttribDivisor ) 
    JUST_LOAD_BY_FN( glVertexAttribP1ui ) 
    JUST_LOAD_BY_FN( glVertexAttribP1uiv ) 
    JUST_LOAD_BY_FN( glVertexAttribP2ui ) 
    JUST_LOAD_BY_FN( glVertexAttribP2uiv ) 
    JUST_LOAD_BY_FN( glVertexAttribP3ui ) 
    JUST_LOAD_BY_FN( glVertexAttribP3uiv ) 
    JUST_LOAD_BY_FN( glVertexAttribP4ui ) 
    JUST_LOAD_BY_FN( glVertexAttribP4uiv ) 

    // gl version 4.0

    JUST_LOAD_BY_FN( glMinSampleShading ) 
    JUST_LOAD_BY_FN( glBlendEquationi ) 
    JUST_LOAD_BY_FN( glBlendEquationSeparatei ) 
    JUST_LOAD_BY_FN( glBlendFunci ) 
    JUST_LOAD_BY_FN( glBlendFuncSeparatei ) 
    JUST_LOAD_BY_FN( glDrawArraysIndirect ) 
    JUST_LOAD_BY_FN( glDrawElementsIndirect ) 
    JUST_LOAD_BY_FN( glUniform1d ) 
    JUST_LOAD_BY_FN( glUniform2d ) 
    JUST_LOAD_BY_FN( glUniform3d ) 
    JUST_LOAD_BY_FN( glUniform4d ) 
    JUST_LOAD_BY_FN( glUniform1dv ) 
    JUST_LOAD_BY_FN( glUniform2dv ) 
    JUST_LOAD_BY_FN( glUniform3dv ) 
    JUST_LOAD_BY_FN( glUniform4dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix2dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix3dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix4dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix2x3dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix2x4dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix3x2dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix3x4dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix4x2dv ) 
    JUST_LOAD_BY_FN( glUniformMatrix4x3dv ) 
    JUST_LOAD_BY_FN( glGetUniformdv ) 
    JUST_LOAD_BY_FN( glGetSubroutineUniformLocation ) 
    JUST_LOAD_BY_FN( glGetSubroutineIndex ) 
    JUST_LOAD_BY_FN( glGetActiveSubroutineUniformiv ) 
    JUST_LOAD_BY_FN( glGetActiveSubroutineUniformName ) 
    JUST_LOAD_BY_FN( glGetActiveSubroutineName ) 
    JUST_LOAD_BY_FN( glUniformSubroutinesuiv ) 
    JUST_LOAD_BY_FN( glGetUniformSubroutineuiv ) 
    JUST_LOAD_BY_FN( glGetProgramStageiv ) 
    JUST_LOAD_BY_FN( glPatchParameteri ) 
    JUST_LOAD_BY_FN( glPatchParameterfv ) 
    JUST_LOAD_BY_FN( glBindTransformFeedback ) 
    JUST_LOAD_BY_FN( glDeleteTransformFeedbacks ) 
    JUST_LOAD_BY_FN( glGenTransformFeedbacks ) 
    JUST_LOAD_BY_FN( glIsTransformFeedback ) 
    JUST_LOAD_BY_FN( glPauseTransformFeedback ) 
    JUST_LOAD_BY_FN( glResumeTransformFeedback ) 
    JUST_LOAD_BY_FN( glDrawTransformFeedback ) 
    JUST_LOAD_BY_FN( glDrawTransformFeedbackStream ) 
    JUST_LOAD_BY_FN( glBeginQueryIndexed ) 
    JUST_LOAD_BY_FN( glEndQueryIndexed ) 
    JUST_LOAD_BY_FN( glGetQueryIndexediv ) 
 
    // gl version 4.1
         
    JUST_LOAD_BY_FN( glReleaseShaderCompiler ) 
    JUST_LOAD_BY_FN( glShaderBinary ) 
    JUST_LOAD_BY_FN( glGetShaderPrecisionFormat ) 
    JUST_LOAD_BY_FN( glDepthRangef ) 
    JUST_LOAD_BY_FN( glClearDepthf ) 
    JUST_LOAD_BY_FN( glGetProgramBinary ) 
    JUST_LOAD_BY_FN( glProgramBinary ) 
    JUST_LOAD_BY_FN( glProgramParameteri ) 
    JUST_LOAD_BY_FN( glUseProgramStages ) 
    JUST_LOAD_BY_FN( glActiveShaderProgram ) 
    JUST_LOAD_BY_FN( glCreateShaderProgramv ) 
    JUST_LOAD_BY_FN( glBindProgramPipeline ) 
    JUST_LOAD_BY_FN( glDeleteProgramPipelines ) 
    JUST_LOAD_BY_FN( glGenProgramPipelines ) 
    JUST_LOAD_BY_FN( glIsProgramPipeline ) 
    JUST_LOAD_BY_FN( glGetProgramPipelineiv ) 
    JUST_LOAD_BY_FN( glProgramUniform1i ) 
    JUST_LOAD_BY_FN( glProgramUniform1iv ) 
    JUST_LOAD_BY_FN( glProgramUniform1f ) 
    JUST_LOAD_BY_FN( glProgramUniform1fv ) 
    JUST_LOAD_BY_FN( glProgramUniform1d ) 
    JUST_LOAD_BY_FN( glProgramUniform1dv ) 
    JUST_LOAD_BY_FN( glProgramUniform1ui ) 
    JUST_LOAD_BY_FN( glProgramUniform1uiv ) 
    JUST_LOAD_BY_FN( glProgramUniform2i ) 
    JUST_LOAD_BY_FN( glProgramUniform2iv ) 
    JUST_LOAD_BY_FN( glProgramUniform2f ) 
    JUST_LOAD_BY_FN( glProgramUniform2fv ) 
    JUST_LOAD_BY_FN( glProgramUniform2d ) 
    JUST_LOAD_BY_FN( glProgramUniform2dv ) 
    JUST_LOAD_BY_FN( glProgramUniform2ui ) 
    JUST_LOAD_BY_FN( glProgramUniform2uiv ) 
    JUST_LOAD_BY_FN( glProgramUniform3i ) 
    JUST_LOAD_BY_FN( glProgramUniform3iv ) 
    JUST_LOAD_BY_FN( glProgramUniform3f ) 
    JUST_LOAD_BY_FN( glProgramUniform3fv ) 
    JUST_LOAD_BY_FN( glProgramUniform3d ) 
    JUST_LOAD_BY_FN( glProgramUniform3dv ) 
    JUST_LOAD_BY_FN( glProgramUniform3ui ) 
    JUST_LOAD_BY_FN( glProgramUniform3uiv ) 
    JUST_LOAD_BY_FN( glProgramUniform4i ) 
    JUST_LOAD_BY_FN( glProgramUniform4iv ) 
    JUST_LOAD_BY_FN( glProgramUniform4f ) 
    JUST_LOAD_BY_FN( glProgramUniform4fv ) 
    JUST_LOAD_BY_FN( glProgramUniform4d ) 
    JUST_LOAD_BY_FN( glProgramUniform4dv ) 
    JUST_LOAD_BY_FN( glProgramUniform4ui ) 
    JUST_LOAD_BY_FN( glProgramUniform4uiv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix2fv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix3fv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix4fv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix2dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix3dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix4dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix2x3fv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix3x2fv )  
    JUST_LOAD_BY_FN( glProgramUniformMatrix2x4fv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix4x2fv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix3x4fv )  
    JUST_LOAD_BY_FN( glProgramUniformMatrix4x3fv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix2x3dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix3x2dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix2x4dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix4x2dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix3x4dv ) 
    JUST_LOAD_BY_FN( glProgramUniformMatrix4x3dv ) 
    JUST_LOAD_BY_FN( glValidateProgramPipeline ) 
    JUST_LOAD_BY_FN( glGetProgramPipelineInfoLog )  
    JUST_LOAD_BY_FN( glVertexAttribL1d  ) 
    JUST_LOAD_BY_FN( glVertexAttribL2d ) 
    JUST_LOAD_BY_FN( glVertexAttribL3d ) 
    JUST_LOAD_BY_FN( glVertexAttribL4d ) 
    JUST_LOAD_BY_FN( glVertexAttribL1dv ) 
    JUST_LOAD_BY_FN( glVertexAttribL2dv ) 
    JUST_LOAD_BY_FN( glVertexAttribL3dv ) 
    JUST_LOAD_BY_FN( glVertexAttribL4dv ) 
    JUST_LOAD_BY_FN( glVertexAttribLPointer ) 
    JUST_LOAD_BY_FN( glGetVertexAttribLdv ) 
    JUST_LOAD_BY_FN( glViewportArrayv ) 
    JUST_LOAD_BY_FN( glViewportIndexedf ) 
    JUST_LOAD_BY_FN( glViewportIndexedfv ) 
    JUST_LOAD_BY_FN( glScissorArrayv ) 
    JUST_LOAD_BY_FN( glScissorIndexed ) 
    JUST_LOAD_BY_FN( glScissorIndexedv ) 
    JUST_LOAD_BY_FN( glDepthRangeArrayv ) 
    JUST_LOAD_BY_FN( glDepthRangeIndexed ) 
    JUST_LOAD_BY_FN( glGetFloati_v ) 
    JUST_LOAD_BY_FN( glGetDoublei_v ) 

    _init = true ;

    return motor::ogl::result::ok ;
}

