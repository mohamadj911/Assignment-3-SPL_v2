package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        // TODO: implement this
        // Added by us
        // you can use any server...
        // String [] args={"2000","tpc"};
        // args[0]="2000";
        // args[1]="reactor";
        if(args[1].equals("tpc")){
        Server.threadPerClient(
            Integer.parseInt(args[0]), // port
                StompProtocolAdapter::new, // protocol factory
                StompMessageEncoderDecoder::new // message encoder decoder factory ///////
        ).serve();

        }else if(args[1].equals("reactor")){
        Server.reactor(
        Runtime.getRuntime().availableProcessors(),
        Integer.parseInt(args[0]), //port
        StompProtocolAdapter::new, //protocol factory
        StompMessageEncoderDecoder::new //message encoder decoder factory
        ).serve();
        }else{
            System.out.print("something wrong!!");
        }
        // Adding end
    }
}
