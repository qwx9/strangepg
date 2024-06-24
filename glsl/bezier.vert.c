char *bezier_vertsh = \
	"#version 430\n"
	"uniform mat4 mvp;\n"
	"in vec2 pos;\n"
	"void\n"
	"main(){\n"
	"	// note: clipping is done in vertex postproc, after the geom shader\n"
	"	gl_Position = mvp * vec4(pos, 8.0, 1.0);\n"
	"}\n";
