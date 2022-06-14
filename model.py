import torch


class GowGameModel(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.embed = torch.nn.Embedding(16 * 1024 * 1024, 8)
        self.state_embed = torch.nn.Embedding(4, 32)
        self.counts_embed = torch.nn.Embedding(200, 16)
        #self.attention = torch.nn.MultiheadAttention(8, 1).cuda()

        self.network = torch.nn.Sequential(
            torch.nn.Linear(8 * 49 + 32 + 16, 256),
            torch.nn.BatchNorm1d(256),
            torch.nn.Linear(256, 64),
            torch.nn.ReLU(),
            torch.nn.Linear(64, 16),
            torch.nn.ReLU(),
            torch.nn.Linear(16, 1)
        ).cuda()

    def forward(self, state, counts, mp):
        x = self.embed(torch.remainder(mp, 16*1024*1024))
        x = x.cuda()
        # x, _ = self.attention(x, x, x, need_weights=False)

        x = torch.flatten(x, 1, 2)

        state = self.state_embed(state).cuda()
        counts = self.counts_embed(counts).cuda()

        x = torch.concat((state, counts, x), 1)
        return self.network(x).reshape(x.shape[0])
