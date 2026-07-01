using System;
using helengine;

static float4 TransformRow(float4 vector, float4x4 matrix) {
    return new float4(
        (vector.X * matrix.M11) + (vector.Y * matrix.M21) + (vector.Z * matrix.M31) + (vector.W * matrix.M41),
        (vector.X * matrix.M12) + (vector.Y * matrix.M22) + (vector.Z * matrix.M32) + (vector.W * matrix.M42),
        (vector.X * matrix.M13) + (vector.Y * matrix.M23) + (vector.Z * matrix.M33) + (vector.W * matrix.M43),
        (vector.X * matrix.M14) + (vector.Y * matrix.M24) + (vector.Z * matrix.M34) + (vector.W * matrix.M44));
}

static float4x4 CreateTransformMatrix(float3 position, float3 scale, float4 orientation) {
    float4x4 rotation;
    float4x4.CreateFromQuaternion(ref orientation, out rotation);
    float4x4 size;
    float4x4.CreateScale(scale.X, scale.Y, scale.Z, out size);
    float4x4 scaleRotation;
    float4x4.Multiply(ref size, ref rotation, out scaleRotation);
    float4x4 translation;
    float4x4.CreateTranslation(ref position, out translation);
    float4x4 transform;
    float4x4.Multiply(ref scaleRotation, ref translation, out transform);
    return transform;
}

void Dump(string label, float4x4 projection) {
    float4 cubeOrientation;
    float4.CreateFromYawPitchRoll(0f, 0f, (float)(18.0 * Math.PI / 180.0), out cubeOrientation);
    float4x4 world = CreateTransformMatrix(new float3(7f, 1f, 0f), new float3(4f, 1f, 2f), cubeOrientation);

    float4 cameraOrientation;
    float4.CreateFromYawPitchRoll(0f, (float)(-20.0 * Math.PI / 180.0), 0f, out cameraOrientation);
    float3 cameraPos = new float3(0f, 8f, 18f);
    float3 cameraForward = float4.RotateVector(new float3(0f, 0f, -1f), cameraOrientation);
    float3 cameraUp = float4.RotateVector(new float3(0f, 1f, 0f), cameraOrientation);
    float3 cameraTarget = cameraPos + cameraForward;
    float4x4 view;
    float4x4.CreateLookAt(ref cameraPos, ref cameraTarget, ref cameraUp, out view);
    float4x4 viewProjection;
    float4x4.Multiply(ref view, ref projection, out viewProjection);
    float4x4 worldViewProjection;
    float4x4.Multiply(ref world, ref viewProjection, out worldViewProjection);

    float3[] vertices = new[] {
        new float3(-0.5f, -0.5f, -0.5f),
        new float3(0.5f, -0.5f, -0.5f),
        new float3(0.5f, 0.5f, -0.5f),
        new float3(-0.5f, 0.5f, -0.5f),
        new float3(-0.5f, -0.5f, 0.5f),
        new float3(0.5f, -0.5f, 0.5f),
        new float3(0.5f, 0.5f, 0.5f),
        new float3(-0.5f, 0.5f, 0.5f)
    };

    Console.WriteLine(label);
    for (int i = 0; i < vertices.Length; i++) {
        float4 clip = TransformRow(new float4(vertices[i].X, vertices[i].Y, vertices[i].Z, 1f), worldViewProjection);
        float ndcX = clip.X / clip.W;
        float ndcY = clip.Y / clip.W;
        Console.WriteLine($"v{i}: ({ndcX:F4}, {ndcY:F4})");
    }
}

float4x4 perspective;
float4x4.CreatePerspectiveFieldOfView((float)Math.PI / 4f, 1280f / 720f, 0.1f, 100f, out perspective);
Dump("perspective", perspective);
float4x4 orthographic;
float4x4.CreateOrthographicOffCenter(-16f, 16f, -9f, 9f, 0.1f, 100f, out orthographic);
Dump("orthographic", orthographic);
