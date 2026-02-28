#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "queue.h"
#include "lib.h"
#include "protocols.h"

/* Routing table */
struct route_table_entry *route_table;
int route_table_len;

/* Mac table*/
struct arp_table_entry *mac_table;
int mac_table_len;

/* Compare function for QSort */
int compare_entries(const void *a, const void *b) {
  	struct route_table_entry *elem1 = (struct route_table_entry *)a;
  	struct route_table_entry *elem2 = (struct route_table_entry *)b;

/* Sort descending after prefix, then after mask */
  	if (elem1->prefix == elem2->prefix) 
    	return elem2->mask - elem1->mask;
 	return elem2->prefix - elem1->prefix;
}

/* Function to get the best route 
   We use QSort for a more efficient way then a simple linear crossing */
struct route_table_entry *get_best_route(uint32_t ip_dest) {
    int left = 0;
    int right = route_table_len - 1;
    struct route_table_entry *best_match = NULL;

	qsort(route_table, route_table_len, sizeof(struct route_table_entry), compare_entries);

	/*we order descending according to the prefix, and then the mask, when we
	have the same prefix, we have to go to the left one element at a time to
	find a more specific entry*/
    while (left <= right) {
        int mid = (left + right)/ 2;
        uint32_t masked_prefix = ip_dest & route_table[mid].mask;

        if (route_table[mid].prefix == masked_prefix) {
			best_match = &route_table[mid];
			    
			while (1) {
        		mid--;
				masked_prefix = ip_dest & route_table[mid].mask;
				if (route_table[mid].prefix > masked_prefix)
					return best_match;

				if(route_table[mid].mask >= best_match->mask)
					best_match = &route_table[mid];  
			}
			
        } else if (route_table[mid].prefix > (ip_dest & route_table[mid].mask))
            left = mid + 1;
        else
            right = mid - 1;
        
    }

    return best_match;
}

/* Function to get Mac entry */
struct arp_table_entry *get_mac_entry(uint32_t given_ip) {
	
	for(int i=0; i<mac_table_len; i++)
		if(mac_table[i].ip == given_ip)
			return &mac_table[i];

	return NULL;
}

/* Funtion that replies an ICMP packet, by type */
void icmp_reply(int interface, char *buf, int reply_type) {
	/* Nedeed headers */
	struct ether_header *eth_hdr = (struct ether_header *) buf;
	struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));
	struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + sizeof(struct ether_header) + sizeof(struct iphdr));


	/* Construct our reply packet */
    char reply_buf[MAX_PACKET_LEN];
    memcpy(reply_buf, buf, MAX_PACKET_LEN);
	int len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);

	
	/* Ethernet header part */
	struct ether_header *reply_eth_hdr = (struct ether_header *)(reply_buf);

	memcpy(reply_eth_hdr->ether_dhost, eth_hdr->ether_shost, 6 * sizeof(uint8_t));
	memcpy(reply_eth_hdr->ether_shost, eth_hdr->ether_dhost, 6 * sizeof(uint8_t));
	reply_eth_hdr->ether_type = eth_hdr->ether_type;


	/* IPv4 header part */
	struct iphdr *reply_ip_hdr = (struct iphdr *)(reply_buf + sizeof(struct ether_header));

	reply_ip_hdr->daddr = ip_hdr->saddr;
	reply_ip_hdr->saddr = ip_hdr->daddr;

	reply_ip_hdr->protocol = 1;	
    reply_ip_hdr->ttl = 64;
    reply_ip_hdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));


	/* ICMP header part*/
	struct icmphdr *reply_icmp_hdr = (struct icmphdr *)(reply_buf + sizeof(struct ether_header) + sizeof(struct iphdr));

	reply_icmp_hdr->type = reply_type;
	reply_icmp_hdr->checksum = 0;
	reply_icmp_hdr->checksum = checksum((uint16_t *)reply_icmp_hdr, sizeof(struct icmphdr));


	/* Send reply */
	send_to_link(interface, reply_buf, len);
}

int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];


	// Do not modify this line
	init(argc - 2, argv + 2);

	route_table = malloc(sizeof(struct route_table_entry) * 100000);
	DIE(route_table == NULL, "memory");

	mac_table = malloc(sizeof(struct arp_table_entry) * 100000);
	DIE(mac_table == NULL, "memory");

	route_table_len = read_rtable(argv[1], route_table);
	mac_table_len = parse_arp_table("arp_table.txt", mac_table);

	while (1) {

		int interface;
		size_t len;

		interface = recv_from_any_link(buf, &len);
		DIE(interface < 0, "recv_from_any_links");

		/* Get needed headers */
		struct ether_header *eth_hdr = (struct ether_header *)(buf);
		struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));
		struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + sizeof(struct ether_header) + sizeof(struct iphdr));
		/* Note that packets received are in network order,
		any header field which has more than 1 byte will need to be conerted to
		host order. For example, ntohs(eth_hdr->ether_type). The oposite is needed when
		sending a packet on the link, */

		if (eth_hdr->ether_type == htons(ETHERTYPE_IP)) {

			/* Verify checksum */
			uint16_t check_aux = ntohs(ip_hdr->check); 
			ip_hdr->check = 0;
			if (checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)) != check_aux){
				printf("checksum diferit\n");
				continue;
				}

			/* Verify if the packet is for the router */
			if(ip_hdr->daddr == inet_addr(get_interface_ip(interface)))
				if(icmp_hdr->type == 8) {
					icmp_reply(interface, buf, 0);
					continue;
				}

			/* Verify ttl */
			if(ip_hdr->ttl <= 1) {
		        icmp_reply(interface, buf, 11);
				continue;
				}

			/* Get best route */
			struct route_table_entry *best_route = get_best_route((uint32_t)ip_hdr->daddr);
			if (best_route == NULL) {
				icmp_reply(interface, buf, 3);
				continue;
				}

		/* Decrement ttl, and recalculate checksum */
		ip_hdr->ttl--;
		ip_hdr->check = 0;
		ip_hdr->check = htons(checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)));
 
		struct arp_table_entry *dmac = get_mac_entry(best_route->next_hop);
		uint8_t smac[6];

		get_interface_mac(best_route->interface, smac);

		memcpy(eth_hdr->ether_dhost, dmac->mac, 6 * sizeof(uint8_t));
		memcpy(eth_hdr->ether_shost, smac, 6 * sizeof(uint8_t));

		send_to_link(best_route->interface, buf, len);
		} else {
			continue;
		}
	}
}
