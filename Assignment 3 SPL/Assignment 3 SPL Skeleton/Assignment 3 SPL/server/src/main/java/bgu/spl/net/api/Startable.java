package bgu.spl.net.api;

import bgu.spl.net.srv.Connections;

public interface Startable<T> {
    void start(int connectionId, Connections<T> connections);
}
