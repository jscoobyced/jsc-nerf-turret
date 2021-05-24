#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>
#include "bt.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

sdp_session_t *register_service()
{
  uint32_t service_uuid_int[] = {0, 0, 0, 0xABCD};
  uint8_t rfcomm_channel = 11;
  const char *service_name = "Roto-Rooter Data Router";
  const char *service_dsc = "An experimental plumbing router";
  const char *service_prov = "Roto-Rooter";

  uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid;
  sdp_list_t *l2cap_list = 0,
             *rfcomm_list = 0,
             *root_list = 0,
             *proto_list = 0,
             *access_proto_list = 0;
  sdp_data_t *channel = 0;

  sdp_record_t *record = sdp_record_alloc();

  // set the general service ID
  sdp_uuid128_create(&svc_uuid, &service_uuid_int);
  sdp_set_service_id(record, svc_uuid);

  // make the service record publicly browsable
  sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
  root_list = sdp_list_append(0, &root_uuid);
  sdp_set_browse_groups(record, root_list);

  // set l2cap information
  sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
  l2cap_list = sdp_list_append(0, &l2cap_uuid);
  proto_list = sdp_list_append(0, l2cap_list);

  // set rfcomm information
  sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
  channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
  rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
  sdp_list_append(rfcomm_list, channel);
  sdp_list_append(proto_list, rfcomm_list);

  // attach protocol information to service record
  access_proto_list = sdp_list_append(0, proto_list);
  sdp_set_access_protos(record, access_proto_list);

  // set the name, provider, and description
  sdp_set_info_attr(record, service_name, service_prov, service_dsc);
  sdp_session_t *session = 0;

  // connect to the local SDP server, register the service record, and
  // disconnect
  session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
  sdp_record_register(session, record, 0);

  // cleanup
  sdp_data_free(channel);
  sdp_list_free(l2cap_list, 0);
  sdp_list_free(rfcomm_list, 0);
  sdp_list_free(root_list, 0);
  sdp_list_free(access_proto_list, 0);

  return session;
}

void findDevice()
{
  uint8_t svc_uuid_int[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0xab, 0xcd};
  uuid_t svc_uuid;
  bdaddr_t target;
  sdp_list_t *response_list = NULL, *search_list, *attrid_list;
  sdp_session_t *session = 0;

  str2ba("B8:27:EB:E5:AD:72", &target);

  // connect to the SDP server running on the remote machine
  session = sdp_connect(BDADDR_ANY, &target, SDP_RETRY_IF_BUSY);

  // specify the UUID of the application we're searching for
  sdp_uuid128_create(&svc_uuid, &svc_uuid_int);
  search_list = sdp_list_append(NULL, &svc_uuid);

  // specify that we want a list of all the matching applications' attributes
  uint32_t range = 0x0000ffff;
  attrid_list = sdp_list_append(NULL, &range);

  // get a list of service records that have UUID 0xabcd
  sdp_service_search_attr_req(session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
  sdp_list_t *r = response_list;

    // go through each of the service records
    for (; r; r = r->next ) {
        sdp_record_t *rec = (sdp_record_t*) r->data;
        sdp_list_t *proto_list;
        
        // get a list of the protocol sequences
        if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
        sdp_list_t *p = proto_list;

        // go through each protocol sequence
        for( ; p ; p = p->next ) {
            sdp_list_t *pds = (sdp_list_t*)p->data;

            // go through each protocol list of the protocol sequence
            for( ; pds ; pds = pds->next ) {

                // check the protocol attributes
                sdp_data_t *d = (sdp_data_t*)pds->data;
                int proto = 0;
                for( ; d; d = d->next ) {
                    switch( d->dtd ) { 
                        case SDP_UUID16:
                        case SDP_UUID32:
                        case SDP_UUID128:
                            proto = sdp_uuid_to_proto( &d->val.uuid );
                            break;
                        case SDP_UINT8:
                            if( proto == RFCOMM_UUID ) {
                                printf("rfcomm channel: %d\n",d->val.int8);
                            }
                            break;
                    }
                }
            }
            sdp_list_free( (sdp_list_t*)p->data, 0 );
        }
        sdp_list_free( proto_list, 0 );

        }

        printf("found service record 0x%x\n", rec->handle);
        sdp_record_free( rec );
    }

    sdp_close(session);
}

void rfcommListen()
{
  struct sockaddr_rc loc_addr = {0}, rem_addr = {0};
  char buf[1024] = {0};
  int s, client, bytes_read;
  socklen_t opt = sizeof(rem_addr);

  // allocate socket
  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // bind socket to port 1 of the first available
  // local bluetooth adapter
  loc_addr.rc_family = AF_BLUETOOTH;
  loc_addr.rc_bdaddr = *BDADDR_ANY;
  loc_addr.rc_channel = (uint8_t)1;
  bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

  ba2str(&loc_addr.rc_bdaddr, buf);
  fprintf(stdout, "local %s\n", buf);
  // put socket into listening mode
  listen(s, 1);

  // accept one connection
  client = accept(s, (struct sockaddr *)&rem_addr, &opt);

  ba2str(&rem_addr.rc_bdaddr, buf);
  fprintf(stderr, "accepted connection from %s\n", buf);
  memset(buf, 0, sizeof(buf));

  // read data from the client
  bytes_read = read(client, buf, sizeof(buf));
  if (bytes_read > 0)
  {
    printf("received [%s]\n", buf);
  }

  // close connection
  close(client);
  close(s);
}