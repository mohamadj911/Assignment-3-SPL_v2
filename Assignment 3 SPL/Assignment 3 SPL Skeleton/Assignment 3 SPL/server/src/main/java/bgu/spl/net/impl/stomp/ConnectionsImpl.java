package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionsImpl<T> implements Connections<T> {
    public ConcurrentHashMap<Integer, ConnectionHandler<T>> Handlers = new ConcurrentHashMap<Integer, ConnectionHandler<T>>();
    public ConcurrentHashMap<String, List<Integer>> topics = new ConcurrentHashMap<String, List<Integer>>();// to save
                                                                                                            // in it the
                                                                                                            // topics in
                                                                                                            // every
                                                                                                            // topic
                                                                                                            // there is
                                                                                                            // a list of
                                                                                                            // connectors
    public ConcurrentHashMap<HashMap<String, String>, Boolean> ConnectsClientsUsers = new ConcurrentHashMap<HashMap<String, String>, Boolean>();// to
                                                                                                                                                // save
                                                                                                                                                // the
                                                                                                                                                // useers
                                                                                                                                                // connecters
    public int handlerid = 0;
    public Integer MSGID = 0;

    public ConnectionsImpl() {
    };

    @Override
    public boolean send(int connectionId, T msg) {
        // TODO Auto-generated method stub
        Handlers.get(connectionId).send(msg);
        return false;
    }

    @Override
    @SuppressWarnings("unchecked")
    public void send(String channel, T msg) {
        // TODO Auto-generated method stub
        List<Integer> list = topics.get(channel);
        for (Integer handlerid : list) {
            String newmsg = ChangeMSGSubscribtion(handlerid.toString(), (String) msg);
            send(handlerid, (T) newmsg);

        }

    }

    // helper function to change the subscribtion if and the msg id
    private String ChangeMSGSubscribtion(String handId, String msg) {
        String ans = "";
        String msgbefore = msg.substring(0, msg.indexOf(':') + 1);
        msg = msg.substring(msg.indexOf(':') + 1);
        int k = msg.indexOf('\n');
        String msgUntil = msgbefore + handId + "\nmessage-id:";
        ans = msgbefore + handId + msg.substring(k + 1);
        msg = msg.substring(k + 1);
        msg = msg.substring(msg.indexOf(":") + 1);
        ans = msgUntil + MSGID + "\n" + msg.substring(msg.indexOf('\n') + 1);
        MSGID += 1;
        return ans;
    }

    @Override
    public void disconnect(int connectionId) {
        // TODO Auto-generated method stub
        ConcurrentHashMap<String, List<Integer>> Topics = topics;
        for (String key : Topics.keySet()) {
            List<Integer> list = Topics.get(key);
            if (list.contains(connectionId)) {
                if (list.size() == 1) {
                    topics.remove((Object)key);
                } else {
                    topics.get(key).remove((Object)connectionId);
                    
                }
            }
        }
        // try {
        //    // Handlers.get(connectionId).close();
        // } catch (IOException e) {
        //     // TODO Auto-generated catch block
        //     e.printStackTrace();
        // }
        Handlers.remove((Object)connectionId);
    }

    public int add(ConnectionHandler<T> handler) {
        int ans = handlerid;
        Handlers.put(handlerid, handler);
        handlerid++;
        return ans;
    }

}