#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "Utilities.h"
#include "Perlin.h"

class Texture {
    public:
        virtual Color value(double u, double v, const Point3& p) const = 0;
};

class Solid_Color : public Texture {
    public:
        Solid_Color() {}
        Solid_Color(Color c) : color_value(c) {}

        Solid_Color(double red, double green, double blue)
          : Solid_Color(Color(red,green,blue)) {}

        virtual Color value(double u, double v, const Vec3& p) const override {
            return color_value;
        }

    private:
        Color color_value;
};

class Checker_Texture : public Texture {
    public:
        Checker_Texture() {}

        Checker_Texture(shared_ptr<Texture> _even, shared_ptr<Texture> _odd)
            : even(_even), odd(_odd) {}

        Checker_Texture(Color c1, Color c2)
            : even(make_shared<Solid_Color>(c1)) , odd(make_shared<Solid_Color>(c2)) {}

        virtual Color value(double u, double v, const Point3& p) const override {
            auto sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
            if (sines < 0)
                return odd->value(u, v, p);
            else
                return even->value(u, v, p);
        }

    public:
        shared_ptr<Texture> odd;
        shared_ptr<Texture> even;
};


class Noise_Texture : public Texture {
    public:
        Noise_Texture() {}
        Noise_Texture(double sc) : scale(sc) {}

        virtual Color value(double u, double v, const Point3& p) const override {
            return Color(1,1,1) * 0.5 * (1 + sin(scale*p.z() + 10*noise.turb(p)));
        }

    public:
        Perlin noise;
        double scale;
};

#endif