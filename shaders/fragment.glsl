uniform vec4 u_DiffuseColour;
uniform vec4 u_LightDirection;
uniform vec3 u_EyePosition;
uniform float u_Shininess;

varying vec3 v_Position;
varying vec3 v_Normal;


void main() {
    vec3 lightDirection = normalize(vec3(-u_LightDirection));
    vec3 normal = normalize(v_Normal);

    // Ambient
    vec4 ambientColour = vec4(0.2, 0.2, 0.2, 1.0) * u_DiffuseColour;

    // Diffuse
    float diffuse = clamp(dot(normal, lightDirection), 0, 1);

    // Reflection
    vec3 reflectionVector = reflect(-lightDirection, normal);
    vec3 eyeVector = normalize(u_EyePosition - v_Position);
    vec3 halfwayDirectionVec = normalize(lightDirection + eyeVector);
    float specular = pow(clamp(dot(normal, halfwayDirectionVec), 0, 1), u_Shininess);

    gl_FragColor = u_DiffuseColour * specular +
                    u_DiffuseColour * diffuse +
                    ambientColour;
}
