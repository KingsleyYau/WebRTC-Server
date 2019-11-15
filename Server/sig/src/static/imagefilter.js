/**
 * 滤镜
 * @type {module.ImageFilter}
 */
class ImageFilter {
    constructor() {
        this.vertexShaderString = [
            "// 图像顶点着色器",
            "// 纹理坐标(输入)",
            "attribute vec4 aPosition;",
            "// 自定义屏幕坐标(输入)",
            "attribute vec4 aTextureCoordinate; ",
            "// 片段着色器(输出)",
            "varying vec2 vTextureCoordinate;",
            "void main() {",
            "	vTextureCoordinate = aTextureCoordinate.xy;",
            "	gl_Position = aPosition;",
            "}"
        ].join("\n");

        this.fragmentShaderString = [
            "// 图像颜色着色器",
            "precision mediump float;",
            "uniform sampler2D uInputTexture;",
            "uniform lowp vec2 uImagePixel;",
            "uniform lowp vec2 uOffet;",
            "// 片段着色器(输入)",
            "varying highp vec2 vTextureCoordinate;",
            "void main() {",
            "	// 绿和蓝右下偏移",
            "	vec4 right = texture2D(uInputTexture, vTextureCoordinate + uImagePixel * uOffet);",
            "	// 红左上偏移",
            "	vec4 left = texture2D(uInputTexture, vTextureCoordinate - uImagePixel * uOffet);",
            "	gl_FragColor = vec4(left.r, right.g, right.b, 1.0);",
            "}"
        ].join("\n");

        this.filterVertex_0 = new Float32Array([
                // GLES世界坐标, 	// GLES纹理坐标
                1, 1, 		    1, 1,
                -1, 1, 		    0, 1,
                -1, -1, 	    0, 0,
                1, 1, 		    1, 1,
                -1, -1, 	    0, 0,
                1, -1, 		    1, 0
            ]
        );

        this.filterVertex_180 = new Float32Array([
                // GLES世界坐标	// GLES纹理坐标
                1, 1, 		    1, 0,
                -1, 1, 		    0, 0,
                -1, -1, 	    0, 1,
                1, 1, 		    1, 0,
                -1, -1, 	    0, 1,
                1, -1, 		    1, 1
            ]
        );
    }

    init(canvas) {
        // this.gl = getWebGLContext(canvas);
        this.gl = canvas.getContext('webgl');
        this.initFilterVertexBuffer(this.filterVertex_180);
        this.initShaders();
    }

    initShaders() {
        var vertexShader = this.loadShader(this.gl.VERTEX_SHADER, this.vertexShaderString);
        var fragmentShader = this.loadShader(this.gl.FRAGMENT_SHADER, this.fragmentShaderString);

        // Create the program object
        this.programObject = this.gl.createProgram();
        this.gl.attachShader(this.programObject, vertexShader);
        this.gl.attachShader(this.programObject, fragmentShader);

        // // Bind attributes
        this.gl.bindAttribLocation(this.programObject, 0, "aPosition");
        this.gl.bindAttribLocation(this.programObject, 1, "aTextureCoordinate");

        // Link the program
        this.gl.linkProgram(this.programObject);
    }

    loadShader(type, shaderSrc) {
        var shader = this.gl.createShader(type);
        // Load the shader source
        this.gl.shaderSource(shader, shaderSrc);
        // Compile the shader
        this.gl.compileShader(shader);
        // Check the compile status
        if (!this.gl.getShaderParameter(shader, this.gl.COMPILE_STATUS) &&
            !this.gl.isContextLost()) {
            var infoLog = this.gl.getShaderInfoLog(shader);
            this.gl.deleteShader(shader);
            return null;
        }
        return shader;
    }

    draw() {
        this.gl.clear(this.gl.COLOR_BUFFER_BIT);

        this.gl.useProgram(this.programObject);

        this.gl.activeTexture(this.gl.TEXTURE0);
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.imgTexture);

        var aPosition = this.gl.getAttribLocation(this.programObject, "aPosition");
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.glVertexBuffer);
        this.gl.vertexAttribPointer(aPosition, 2, this.gl.FLOAT, false, 16, 0);
        this.gl.enableVertexAttribArray(aPosition);

        var aTextureCoordinate = this.gl.getAttribLocation(this.programObject, "aTextureCoordinate");
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.glVertexBuffer);
        this.gl.vertexAttribPointer(aTextureCoordinate, 2, this.gl.FLOAT, false, 16, 8);
        this.gl.enableVertexAttribArray(aTextureCoordinate);

        this.uInputTexture = this.gl.getUniformLocation(this.programObject, "uInputTexture");
        this.uImagePixel = this.gl.getUniformLocation(this.programObject, "uImagePixel");
        this.uOffet = this.gl.getUniformLocation(this.programObject, "uOffet");

        // 填充着色器-颜色采样器, 将纹理单元GL_TEXTURE0绑元定到采样器
        this.gl.uniform1i(this.uInputTexture, 0);

        this.gl.uniform2f(this.uImagePixel, 1 / 720.0, 1 / 1080.0);

        var num = Math.random() * 10 % 5;
        this.gl.uniform2f(this.uOffet, num, num);

        this.gl.drawArrays(this.gl.TRIANGLES, 0, 6);
    }

    initFilterVertexBuffer(filterVertex) {
        // 创建着色器内存
        this.glVertexBuffer = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.glVertexBuffer);
        this.gl.bufferData(this.gl.ARRAY_BUFFER, filterVertex, this.gl.STATIC_DRAW);
    }

    loadImage(url) {
        this.imgTexture = this.gl.createTexture();
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.imgTexture);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_S, this.gl.CLAMP_TO_EDGE);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_T, this.gl.CLAMP_TO_EDGE);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR);

        var img = new Image(640, 480);
        img.onload = function() {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.imgTexture);
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, img);
        }.bind(this);
        img.src = url;
    }
    genPixelTexture() {
//         // 创建新纹理
//         int textureId[] = new int[1];
//         GLES20.glGenTextures(1, textureId, 0);
//
//         // 绑定纹理
//         this.gl.glBindTexture(GLES20.GL_TEXTURE_2D, textureId[0]);
//
//         /**
//          * GL_LINEAR 不失真, 但会模糊
//          * GL_NEAREST 失真, 但不会模糊
//          *
//          * GL_REPEAT 平铺
//          * GL_MIRRORED_REPEAT 镜像平铺
//          * GL_CLAMP_TO_EDGE 超出部分变为条纹
//          * GL_CLAMP_TO_BORDER 居中
//          */
//         // 设置纹理参数
//         GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
//             GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
//         GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
//             GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
//         GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
//             GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
//         GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
//             GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
//
//         // 解除纹理绑定
//         GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
//
// //        Log.d(LSConfig.TAG, String.format("LSImageFilter::genPixelTexture( textureId : %d )", textureId[0]));
//
//         String method = String.format("genPixelTexture( textureId : %d )", textureId[0]);
//         checkGLError(method, null);
//
//         return textureId;
    }
}