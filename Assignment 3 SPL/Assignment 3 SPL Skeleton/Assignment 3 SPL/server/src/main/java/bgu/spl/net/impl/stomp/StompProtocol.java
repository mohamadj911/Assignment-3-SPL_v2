package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

public class StompProtocol implements StompMessagingProtocol<String> {
    public boolean connected = true;
    private HashMap<Integer, String> SubscribtionMap = new HashMap<Integer, String>(); // or in handler
    private HashMap<String, String> temp;
    private String msg;
    private int ConnectionId;
    private ConnectionsImpl<String> Connections;
    private boolean shouldTerminate;
    private String UserName;
    private String Passcode;
    private boolean ConnectClient = false;

    @Override
    public void start(int connectionId, Connections<String> connections) {
        // TODO Auto-generated method stub
        this.Connections = (ConnectionsImpl<String>) connections;
        this.ConnectionId = connectionId;
        this.shouldTerminate=false;
    }

    @Override
    public void process(String message) { // I receive frome the client exact string he send
        msg = message;
        String command = "";
        for (int i = 0; i < message.length(); i++) {
            if (message.charAt(i) == '\n') {
                command = message.substring(0, i);
                break;
            }
        }
        String response = SolveReplyCommand(command);
        if (response != null && !response.isEmpty()) {
            Connections.send(ConnectionId, response);
        }
    }

    @Override
    public boolean shouldTerminate() {
        // TODO Auto-generated method stub
        return shouldTerminate;
    }

    public String SolveReplyCommand(String command) {
        // System.out.println(command);
        String ans = "";
        if ("CONNECT".equals(command)) {
            ans = CONNECT();
        } else if ("SUBSCRIBE".equals(command)) {
            ans = SUBSCRIBE();
        } else if ("UNSUBSCRIBE".equals(command)) {
            ans = UNSUBSCRIBE();
        } else if ("SEND".equals(command)) {
            ans = MESSAGE();
        } else if ("SUMMARY".equals(command)) {
            ans = SUMMARY();
        } else if ("DISCONNECT".equals(command)) {
            ans = DISCONNECT();
        }else if("ERR0R".equals(command)){
            String msgid = Connections.MSGID.toString();
            Connections.MSGID += 1;
            /////we should close the socket of the server
            return ERROR("SEND", "Wrong Copy Path\nOr You Aren't Member In The Channel\n\n", msgid);
        } 
        else if ("CLOSE".equals(command)) {
            disconnect();
            return "";
        }
        return ans;
    }

    private String GetUserName() {
        msg = msg.substring(msg.indexOf(':') + 1);
        msg = msg.substring(msg.indexOf(':') + 1);
        msg = msg.substring(msg.indexOf(':') + 1);
        return msg.substring(0, msg.indexOf('\n'));
    }

    private String GetPasscode() {
        msg = msg.substring(msg.indexOf(':') + 1);
        msg = msg.substring(msg.indexOf(':') + 1);
        msg = msg.substring(msg.indexOf(':') + 1);
        msg = msg.substring(msg.indexOf(':') + 1);
        return msg.substring(0, msg.indexOf('\n'));
    }

    private boolean CheckIfExsist(String userName){
        for(HashMap<String, String> User:Connections.ConnectsClientsUsers.keySet()){
            if(User.containsKey(userName)){
                temp=User;
                return true;
            }
        }
        return false;
    }
    public String CONNECT(){
        ///errror if login 
        String message=msg;
       if(!ConnectClient){
        UserName=GetUserName();
        Passcode=GetPasscode();
        boolean Exsist=CheckIfExsist(UserName);
        boolean foundJustTheName=JustTheUser(UserName,Passcode);
        if(foundJustTheName){
            return ERROR("PASSWORD",message,"-0-");
        }
        if(Exsist){
            if(Connections.ConnectsClientsUsers.get(temp)){//the user is exsist and its already logged in
                return ERROR("CONNECT", message, "-0-");
            }else{
                Connections.ConnectsClientsUsers.put(temp, true); //the user is exist and not logged in already and the password match
                ConnectClient=true;
                return "CONNECTED\nversion:1.2"+"\n\nLogin Sucssfully\n\n";
            }
        }else{ ///new user (now we will save it)
           ConnectClient=true;
           HashMap<String,String> hash=new HashMap<>();
           hash.put(UserName, Passcode);
           temp=hash;
            Connections.ConnectsClientsUsers.put(hash, true);
            return "CONNECTED\nversion:1.2"+'\n'+'\n';
      }
   }else{
       return ERROR("CONNECT", message, "-0-");
    }
   }
    private boolean JustTheUser(String UserName, String Passcode) {
        for(HashMap<String, String> User:Connections.ConnectsClientsUsers.keySet()){
            if(User.containsKey(UserName)){
                if(User.get(UserName).equals(Passcode)){
                    return false;
                }else return true;
            }
        }
        return false;
    }
    public String SUBSCRIBE() {
        synchronized (Connections) {
            String destination = "";
            String id = "";
            String receiptid = "";
            String message = msg;
            first: for (int com = 0; com < msg.length(); com++) {
                if (msg.charAt(com) == '\n') {
                    for (int des = msg.indexOf(':') + 1; des < msg.length(); des++) {
                        if (msg.charAt(des) != '\n') {
                            destination = destination + msg.charAt(des);
                        } else {
                            msg = msg.substring(des);
                            for (int ID = msg.indexOf(':') + 1; ID < msg.length(); ID++) {
                                if (msg.charAt(ID) != '\n') {
                                    id = id + msg.charAt(ID);
                                } else {
                                    msg = msg.substring(ID);
                                    for (int rec = msg.indexOf(':') + 1; rec < msg.length(); rec++) {
                                        if (msg.charAt(rec) != '\n') {
                                            receiptid = receiptid + msg.charAt(rec);
                                        } else {
                                            break first;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }
            Integer Id = Integer.parseInt(id);
            boolean aa = true;
            for (Integer key : SubscribtionMap.keySet()) {
                if (SubscribtionMap.get(key).equals(destination)) {
                    aa = false;
                }
            }
            if (aa) {
                // update the submap
                SubscribtionMap.put(Id, destination);
                // update the topics in connectins sengleton
                if (Connections.topics.get(destination) != null) {
                    Connections.topics.get(destination).add(ConnectionId);
                } else {
                    List<Integer> list = new LinkedList<Integer>();
                    list.add(ConnectionId);
                    Connections.topics.put(destination, list);
                }
                return "RECEIPT\nreceipt-id:" + receiptid + '\n' + '\n';
            } else {
                return ERROR("SUBSCRIBE", message, receiptid);
            }
        }
    }

    public String UNSUBSCRIBE() {
        String subid = "";
        String receiptid = "";
        String message = msg;
        first: for (int com = 0; com < msg.length(); com++) {
            if (msg.charAt(com) == '\n') {
                for (int ID = msg.indexOf(':') + 1; ID < msg.length(); ID++) {
                    if (msg.charAt(ID) != '\n') {
                        subid = subid + msg.charAt(ID);
                    } else {
                        msg = msg.substring(ID);
                        for (int rec = msg.indexOf(':') + 1; rec < msg.length(); rec++) {
                            if (msg.charAt(rec) != '\n') {
                                receiptid = receiptid + msg.charAt(rec);
                            } else {
                                break first;
                            }
                        }
                    }
                }
            }
        }
        int Id = Integer.parseInt(subid);
        if (SubscribtionMap.get(Id) != null) {
            String topic = SubscribtionMap.get(Id);
            SubscribtionMap.remove((Object)Id);
            Connections.topics.get(topic).remove((Object) ConnectionId);
        } else {
            return ERROR("UNSUBSCRIBE", message, receiptid);
        }
        return "RECEIPT\nreceipt-id:" + receiptid + '\n' + '\n';

    }

    public String SUMMARY() {
        return "{file} {name} {user}" + '\n' + '\n';
    }

    public String DISCONNECT() {
        String receiptid = "";
        int from = msg.indexOf(':');
        for (int i = from + 1; i < msg.length(); i++) {
            if (msg.charAt(i) != '\n') {
                receiptid = receiptid + msg.charAt(i);
            } else {
                break;
            }
        }
        return "\nRECEIPT\nreceipt-id:" + receiptid + '\n' + '\n';
    }
    public String ERROR(String problem,String message,String recipt){
        String ans="";
        if("UNSUBSCRIBE".equals(problem)){
            ///update the recipt
            ans="ERROR\nreceipt-id:"+recipt+"\nmessage: "+problem+"\nthe problem:\n- - - - -\n\n";
            ans+=message+"- - - - -\nDescription: Unsubscribe for undefine Subscription topic name";
        }
        else if ("SUBSCRIBE".equals(problem)){
            ans="ERROR\nreceipt-id:"+recipt+"\nmessage: "+problem+"\nthe problem:\n- - - - -\n\n";
            ans+=message+"- - - - -\nDescription: Subscribe to the same Channel";
        }
        else if("SEND".equals(problem)){
            ans="ERROR\nreceipt-id:"+recipt+"\nmessage: "+problem+"\nthe problem:\n- - - - -\n\n";
            ans+=message+"- - - - -\nDescription: Report is Wrong!!";
        }
        else if ("CONNECT".equals(problem)){
            ans="ERROR\nreceipt-id:"+recipt+"\nmessage: "+problem+"\nthe problem:\n- - - - -\n\n";
            ans+=message+"- - - - -\nDescription: The User Is Already logged in";
        }else if("PASSWORD".equals(problem)){
            ans="ERROR\nreceipt-id:"+recipt+"\nmessage: "+"CONNECT"+"\nthe problem:\n- - - - -\n\n";
            ans+=message+"- - - - -\nDescription: Wrong password!! ";
        }
        return ans;
    }
    public void disconnect() {
        
        SubscribtionMap.clear();
        if(ConnectClient){
         ConnectClient = false;
        Connections.ConnectsClientsUsers.put(temp, false);
        shouldTerminate=true;
        // try {
        //     Connections.Handlers.get(ConnectionId).close();
        // } catch (IOException e) {
        //     // TODO Auto-generated catch block
        //     e.printStackTrace();
        // }
    }
        Connections.disconnect(ConnectionId);
    }

    public String MESSAGE() {
        String destination = "";
        String body = "";
        String ans = "";
        String subid = "";
        String msgid = Connections.MSGID.toString();
        Connections.MSGID += 1;
        int i = msg.indexOf('\n');
        String message = msg;
        msg = msg.substring(i + 1);
        int index = msg.indexOf(":");
        for (int des = index + 1; des < msg.indexOf('\n'); des++) {
            destination = destination + msg.charAt(des);
        }
        body = body(msg);
        subid = findsubid(destination, message, msgid);
        ans = "MESSAGE\nsubscribtion:" + subid + '\n' + "message-id:" + msgid + "\n" + body;
        Connections.send(destination, ans);
        return ans;
    }

    public String body(String str) {
        str.substring(str.indexOf('\n') + 1);
        str.substring(str.indexOf('\n') + 1);
        return str;
    }

    private String findsubid(String destination, String message, String msgid) {
        for (Integer key : SubscribtionMap.keySet()) {
            if (SubscribtionMap.get(key).equals(destination)) {
                return key.toString();
            }
        }
        return ERROR("SEND", message, msgid);
    }

}