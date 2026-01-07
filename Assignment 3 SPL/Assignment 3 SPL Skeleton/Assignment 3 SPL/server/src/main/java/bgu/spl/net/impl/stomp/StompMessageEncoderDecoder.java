package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class StompMessageEncoderDecoder implements MessageEncoderDecoder<String> {

    private final ByteArrayOutputStream buffer = new ByteArrayOutputStream();

    @Override
    public String decodeNextByte(byte nextByte) {
        if (nextByte == '\0') {
            String frame = buffer.toString(StandardCharsets.UTF_8);
            buffer.reset();
            return frame;
        }
        buffer.write(nextByte);
        return null;
    }

    @Override
    public byte[] encode(String message) {
        byte[] data = message.getBytes(StandardCharsets.UTF_8);
        byte[] framed = Arrays.copyOf(data, data.length + 1);
        framed[framed.length - 1] = '\0';
        return framed;
    }
}
