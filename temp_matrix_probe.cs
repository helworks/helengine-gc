using System;
using helengine;

float4 rotation = default;
float4.CreateFromYawPitchRoll(0f, 0f, (float)(18.0 * Math.PI / 180.0), out rotation);
float4x4 rotationMatrix;
float4x4.CreateFromQuaternion(ref rotation, out rotationMatrix);
float4x4 scaleMatrix;
float4x4.CreateScale(4f, 1f, 2f, out scaleMatrix);
float4x4 scaleRotation;
float4x4.Multiply(ref scaleMatrix, ref rotationMatrix, out scaleRotation);

Console.WriteLine($"row1=({scaleRotation.M11:F6}, {scaleRotation.M12:F6}, {scaleRotation.M13:F6})");
Console.WriteLine($"row2=({scaleRotation.M21:F6}, {scaleRotation.M22:F6}, {scaleRotation.M23:F6})");
Console.WriteLine($"row3=({scaleRotation.M31:F6}, {scaleRotation.M32:F6}, {scaleRotation.M33:F6})");

float dot12 = (scaleRotation.M11 * scaleRotation.M21) + (scaleRotation.M12 * scaleRotation.M22) + (scaleRotation.M13 * scaleRotation.M23);
float dot13 = (scaleRotation.M11 * scaleRotation.M31) + (scaleRotation.M12 * scaleRotation.M32) + (scaleRotation.M13 * scaleRotation.M33);
float dot23 = (scaleRotation.M21 * scaleRotation.M31) + (scaleRotation.M22 * scaleRotation.M32) + (scaleRotation.M23 * scaleRotation.M33);
Console.WriteLine($"dot12={dot12:F6} dot13={dot13:F6} dot23={dot23:F6}");
