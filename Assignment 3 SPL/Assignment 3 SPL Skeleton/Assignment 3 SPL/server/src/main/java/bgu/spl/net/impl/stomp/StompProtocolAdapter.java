package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.Startable;
import bgu.spl.net.srv.Connections;

/**
 * Adapter that lets StompProtocol be used where a MessagingProtocol is required.
 * It forwards start to the wrapped protocol and returns null from process since
 * StompProtocol sends via the Connections object directly.
 */
public class StompProtocolAdapter implements MessagingProtocol<String>, Startable<String> {

    private final StompProtocol delegate;

    public StompProtocolAdapter() {
        this.delegate = new StompProtocol();
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {
        delegate.start(connectionId, connections);
    }

    @Override
    public String process(String msg) {
        delegate.process(msg);
        return null; // responses are sent by the delegate through Connections
    }

    @Override
    public boolean shouldTerminate() {
        return delegate.shouldTerminate();
    }
}
