#pragma once
#include <numbers>

namespace ksmaudio::AudioEffect::detail
{
    template <typename T>
    class BiquadFilter
    {
    private:
        T m_a0 = T{ 1 };
        T m_a1 = T{ 0 };
        T m_a2 = T{ 0 };
        T m_b0 = T{ 1 };
        T m_b1 = T{ 0 };
        T m_b2 = T{ 0 };
        T m_input1 = T{ 0 };
        T m_input2 = T{ 0 };
        T m_output1 = T{ 0 };
        T m_output2 = T{ 0 };

    public:
        BiquadFilter() = default;

        T process(T input)
        {
            const T output
                = m_b0 / m_a0 * input
                + m_b1 / m_a0 * m_input1
                + m_b2 / m_a0 * m_input2
                - m_a1 / m_a0 * m_output1
                - m_a2 / m_a0 * m_output2;

            m_input2 = m_input1;
            m_input1 = input;
            m_output2 = m_output1;
            m_output1 = output;

            return output;
        }

        void setLowPassFilter(T freq, T q, T sampleRate)
        {
            const T omega = std::numbers::pi_v<T> * 2 * freq / sampleRate;
            const T alpha = std::sin(omega) / (q * 2);

            const T cosOmega = std::cos(omega);
            m_a0 = T{ 1 } + alpha;
            m_a1 = -T{ 2 } * cosOmega;
            m_a2 = T{ 1 } - alpha;
            m_b0 = (T{ 1 } - cosOmega) / 2;
            m_b1 = T{ 1 } - cosOmega;
            m_b2 = (T{ 1 } - cosOmega) / 2;
        }

        void setLowShelfFilter(T freq, T q, T gain, T sampleRate)
        {
            const T omega = std::numbers::pi_v<T> * 2 * freq / sampleRate;
            const T A = std::pow(T{ 10 }, gain / 40);
            const T beta = std::sqrt(A) / q;

            const T sinOmega = std::sin(omega);
            const T cosOmega = std::cos(omega);
            m_a0 = (A + T{ 1 }) + (A - T{ 1 }) * cosOmega + beta * sinOmega;
            m_a1 = -T{ 2 } * ((A - T{ 1 }) + (A + T{ 1 }) * cosOmega);
            m_a2 = (A + T{ 1 }) + (A - T{ 1 }) * cosOmega - beta * sinOmega;
            m_b0 = A * ((A + T{ 1 }) - (A - T{ 1 }) * cosOmega + beta * sinOmega);
            m_b1 = T{ 2 } * A * ((A - T{ 1 }) - (A + T{ 1 }) * cosOmega);
            m_b2 = A * ((A + T{ 1 }) - (A - T{ 1 }) * cosOmega - beta * sinOmega);
        }

        void setHighPassFilter(T freq, T q, T sampleRate)
        {
            const T omega = std::numbers::pi_v<T> * 2 * freq / sampleRate;
            const T alpha = std::sin(omega) / (q * 2);

            const T cosOmega = std::cos(omega);
            m_a0 = T{ 1 } + alpha;
            m_a1 = -T{ 2 } * cosOmega;
            m_a2 = T{ 1 } - alpha;
            m_b0 = (T{ 1 } - cosOmega) / 2;
            m_b1 = -T{ 1 } - cosOmega;
            m_b2 = (T{ 1 } - cosOmega) / 2;
        }

        void setHighShelfFilter(T freq, T q, T gain, T sampleRate)
        {
            const T omega = std::numbers::pi_v<T> * 2 * freq / sampleRate;
            const T A = std::pow(T{ 10 }, gain / 40);
            const T beta = std::sqrt(A) / q;

            const T sinOmega = std::sin(omega);
            const T cosOmega = std::cos(omega);
            m_a0 = (A + T{ 1 }) - (A - T{ 1 }) * cosOmega + beta * sinOmega;
            m_a1 = T{ 2 } *((A - T{ 1 }) - (A + T{ 1 }) * cosOmega);
            m_a2 = (A + T{ 1 }) - (A - T{ 1 }) * cosOmega - beta * sinOmega;
            m_b0 = A * ((A + T{ 1 }) + (A - T{ 1 }) * cosOmega + beta * sinOmega);
            m_b1 = -T{ 2 } * A * ((A - T{ 1 }) + (A + T{ 1 }) * cosOmega);
            m_b2 = A * ((A + T{ 1 }) + (A - T{ 1 }) * cosOmega - beta * sinOmega);
        }

        void setPeakingFilter(T freq, T q, T gain, T sampleRate)
        {
            const T omega = std::numbers::pi_v<T> * 2 * freq / sampleRate;
            const T alpha = std::sin(omega) * std::sinh(std::log(T{ 2 }) / 2 * q * omega / std::sin(omega));
            const T A = std::pow(T{ 10 }, gain / 40);

            const T cosOmega = std::cos(omega);
            m_a0 = T{ 1 } + alpha / A;
            m_a1 = -T{ 2 } * cosOmega;
            m_a2 = T{ 1 } - alpha / A;
            m_b0 = T{ 1 } + alpha * A;
            m_b1 = -T{ 2 } * cosOmega;
            m_b2 = T{ 1 } - alpha * A;
        }

        void setAllPassFilter(T freq, T q, T sampleRate)
        {
            const T omega = std::numbers::pi_v<T> * 2 * freq / sampleRate;
            const T alpha = std::sin(omega) / (q * 2);

            const T cosOmega = std::cos(omega);
            m_a0 = T{ 1 } + alpha;
            m_a1 = -T{ 2 } * cosOmega;
            m_a2 = T{ 1 } - alpha;
            m_b0 = T{ 1 } - alpha;
            m_b1 = -T{ 2 } * cosOmega;
            m_b2 = T{ 1 } + alpha;
        }
    };
}
